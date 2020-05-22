#include <windows.h>

//----------------------------------Direct3Dを使うためのライブラリ----------------------------------//
#include <d3d11_4.h>
#pragma comment(lib,"d3d11.lib")

//-------------------------------------------ライブラリ--------------------------------------------//
#include <string>
#include <vector>
#include <fstream>
#include <DirectXMath.h>
using namespace DirectX;

//----------------------------------先に宣言------------------------------------//
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
std::shared_ptr<std::vector<char>> LoadBinaryData(const std::wstring &filename);

//----------------------------------解放マクロ----------------------------------//
//"\"を入れたら改行できるが基本一行
#define RELEASE(x) \
do{\
	if (x) {\
			(x)->Release(); \
			(x) = nullptr; \
	}\
}while (0);\


//----------------------------------グローバル変数----------------------------------//
HWND						g_hwnd;									//ウィンドウハンドル
IDXGISwapChain				*g_pSwapChain = nullptr;				//スワップチェイン
ID3D11Device				*g_pd3dDevice = nullptr;				//デバイス
ID3D11DeviceContext			*g_pImmediateContext = nullptr;			//コンテキスト
ID3D11RenderTargetView		*g_renderTargetView = nullptr;			//レンダーターゲットビュー
ID3D11VertexShader			*g_pVertexShader = nullptr;				//バーテックスシェーダー
ID3D11PixelShader			*g_pPixelShader = nullptr;				//ピクセルシェーダー
ID3D11InputLayout			*g_pInputLayout = nullptr;				//インプットレイアウト
ID3D11Buffer				*g_pVertexBuffer = nullptr;				//バーテックスバッファ
ID3D11Buffer				*g_pIndexBuffer = nullptr;				//インデックスバッファ
ID3D11Buffer				*g_pConstantBuffer = nullptr;			//コンスタントバッファ

//----------------------------------頂点データ----------------------------------//
struct Vertex
{
	//float x, y, z;
	XMFLOAT3 position;  //頂点座標
	XMFLOAT3 normal;    //法線ベクトル
};

//----------------コンスタントバッファの元データ(16バイト区切り)---------------------//
struct ConstantBuffer
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;

	//16区切りにしないといけないので
	//float time;//4byte
	//float dummy[3]//12byte
	//ダミー追加して合計16byteになるようにしないといけない
};

bool isRunning(MSG* msg) {
	if(PeekMessage(msg, 0, 0, 0, PM_REMOVE)){
		if(msg->message == WM_QUIT) {
			return false;
		}
			//メッセージを送信
			DispatchMessage(msg);
	}
	return true;
}

///-------------------------------- <summary>
/// Main関数の代わり
/// WINAPIをつけること
/// WINAPI		:Win32APIを呼び出す時の規約
/// [引数]
/// HINSTANCE	:インスタンスハンドル、一部のAPI関数を呼び出す時に使う
/// HINSTANCE	:Win16の産物、今は常にNULL
/// LPSTR		:コマンドラインから受け取った引数
/// int			:初期表示方法を指定できる
/// </summary>-------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	//	[ウィンドウの作り方]
	//1	ウィンドウクラスの作成
	//2	作ったものをWindowsに登録
	//3	ウィンドウ生成


	//--------------------ウィンドウクラスのデータ作成--------------------//
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;			//Windowスタイル
	wc.lpfnWndProc = WindowProc;				//WindowProcedure
	wc.cbClsExtra = 0;							//EX領域、今のところ意味なし
	wc.cbWndExtra = 0;							//EX領域、今のところ意味なし
	wc.hInstance = GetModuleHandle(NULL);		//アプリケーションインスタンスを取得
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);	//Windowアイコン
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);	//Window内で使用されるカーソル
	wc.hbrBackground = 0;						//クライアント領域の背景
	wc.lpszMenuName = NULL;						//メニューネーム
	wc.lpszClassName = L"Title";				//ウィンドウクラスの名前

	//--------------------ウィンドウクラスの登録-------------------------//
	RegisterClass(&wc);
	
	//------------------------ウィンドウの生成--------------------------//
	g_hwnd = CreateWindow(L"Title", L"Title", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 640, 480, 0, 0, wc.hInstance, 0);


	//------DirectXを初期化する------------
	//1.Direct3Dオブジェクトを生成する
	//2.デバイスコンテキストを生成する
	//3.スワップチェインを生成する
	//4.バックバッファと取得してレンダーターゲットビューを生成する
	//--------リソース準備-----------------
	//5.頂点シェーダを生成する
	//6.ピクセルシェーダを生成する
	//7.インプットレイアウトを生成する
	//8.頂点(バーテックス)バッファを生成する
	//9.インデックスバッファを生成する
	//10.コンスタントバッファを生成する
	//------------描画--------------------
	//11.画面をクリア
	//12.バックバッファにポリゴン生成
	//13.バックバッファとフロントバッファを入れ替える


	/// --------------------------------<summary>
	/// sd.BufferCount							//バックバッファの数
	/// sd.BufferDesc.Width						//横解像度
	/// sd.BufferDesc.Height					//縦解像度
	/// sd.BufferDesc.Format					//描画先形式
	/// sd.BufferDesc.RefreshRate.Numerator		//リフレッシュレートの分子
	/// sd.BufferDesc.RefreshRate.Denominator	//リフレッシュレートの分母
	/// sd.BufferUsage							//何をするためのものなのか
	/// sd.OutputWindow							//ウィンドウハンドル
	/// sd.SampleDesc.Count						//
	/// sd.SampleDesc.Quality					//MSAA設定
	/// sd.Windowed								//ウィンドウモードか
	/// </summary>--------------------------------

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));//0で初期化
	sd.BufferCount = 1;
	sd.BufferDesc.Width = 640;
	sd.BufferDesc.Height = 480;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;


	/// --------------------------------<summary>
	/// D3D11CreateDeviceAndSwapChain(
	/// IDXGIAdapter,				// どのビデオアダプタを使用するか？既定ならばnullptrで、IDXGIAdapterのアドレスを渡す.
	///	D3D_DRIVER_TYPE,			// ドライバのタイプを渡す。これ以外は基本的にソフトウェア実装で、どうしてもという時やデバグ用に用いるべし.
	///	HMODULE,					// 上記をD3D_DRIVER_TYPE_SOFTWAREに設定した際に、その処理を行うDLLのハンドルを渡す。それ以外を指定している際には必ずnullptrを渡す.
	///	UINT,						// 何らかのフラグを指定する。詳しくはD3D11_CREATE_DEVICE列挙型で検索検索ぅ.
	///	const D3D_FEATURE_LEVEL,	// 実はここでD3D_FEATURE_LEVEL列挙型の配列を与える。nullptrにすることで上記featureと同等の内容の配列が使用される.
	///	UINT,						// 上記引数で、自分で定義した配列を与えていた場合、その配列の要素数をここに記述する.
	///	UINT,						// SDKのバージョン。必ずこの値->D3D11_SDK_VERSION
	///	const DXGI_SWAP_CHAIN_DESC,	// DXGI_SWAP_CHAIN_DESC構造体のアドレスを設定する。ここで設定した構造愛に設定されているパラメータでSwapChainが作成される.
	///	IDXGISwapChain,				// 作成が成功した場合に、そのSwapChainのアドレスを格納するポインタ変数へのアドレス。ここで指定したポインタ変数経由でSwapChainを操作する.
	///	ID3D11Device,				// 上記とほぼ同様で、こちらにはDeviceのポインタ変数のアドレスを設定する.
	///	D3D_FEATURE_LEVEL,			// 実際に作成に成功したD3D_FEATURE_LEVELを格納するためのD3D_FEATURE_LEVEL列挙型変数のアドレスを設定する.
	///	ID3D11DeviceContext			// SwapChainやDeviceと同様に、こちらにはContextのポインタ変数のアドレスを設定する.
	/// </summary>--------------------------------

	D3D_FEATURE_LEVEL  FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0;
	UINT               numFeatureLevelsRequested = 1;
	D3D_FEATURE_LEVEL  FeatureLevelsSupported;

	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		&FeatureLevelsRequested,
		numFeatureLevelsRequested,
		D3D11_SDK_VERSION,
		&sd,
		&g_pSwapChain,
		&g_pd3dDevice,
		&FeatureLevelsSupported,
		&g_pImmediateContext);
	if (FAILED(hr)) {
		MessageBox(0,L"D3D11CreateDeviceAndSwapChain Failed!",0,0);
	}

	//--------------------バックバッファ取得--------------------//
	ID3D11Texture2D *backbuffer = nullptr;
	/// GetBuffer(UINT,REFID,void**)
	/// UINT	:バッファインデックス(基本0)
	/// REFID	:取得するバッファのインターフェースID
	/// void**	:バッファの取得先
	/// [void**について]
	/// 汎用ポインタ、なんでも入るがキャスト必須
	hr = g_pSwapChain->GetBuffer(0,__uuidof(ID3D11Texture2D),(void**)&backbuffer);
	if (FAILED(hr)) {
		MessageBox(0, L"GetBuffer Failed!", 0, 0);
	}

	//----------------レンダーターゲットビュー作成----------------//
	/// CreateRenderTargetView(ID3D11Resource*,const D3D11_RENDER_TARGET_VIEW_DESC *,ID3D11RenderTargetView**);
	/// ID3D11Resource					:作成するバッファのリソース
	/// D3D11_RENDER_TARGET_VIEW_DESC	:作成するViewの設定,nullptrでデフォルト設定
	/// ID3D11RenderTargetView			:作成されたものの収納する所のポインタ
	hr = g_pd3dDevice->CreateRenderTargetView(backbuffer,nullptr,&g_renderTargetView);
	if (FAILED(hr)) {
		MessageBox(0, L"CreateRenderTargetView Failed!", 0, 0);
	}

	//使い終わったので解放ですね
	RELEASE(backbuffer);

	//--------------------頂点シェーダーの読み込みと生成作成--------------------//
	/// CreateVertexShader
	///	const void         *pShaderBytecode		:コンパイル済みシェーダのバイナリデータ
	///	SIZE_T             BytecodeLength		:バイナリデータサイズ(Byte)
	///	ID3D11ClassLinkage *pClassLinkage		:クラスリンケージインターフェイスへのポインタ(NULL可能)
	///	ID3D11VertexShader **ppVertexShader		:生成されたシェーダー

	auto vsByteCode = LoadBinaryData(L"VertexShader.cso");
	hr = g_pd3dDevice->CreateVertexShader(
	&vsByteCode->front(),
	vsByteCode->size(),
	0,
	&g_pVertexShader);
	if (FAILED(hr)) {
		MessageBox(0, L"CreateVertexShader Failed!", 0, 0);
	}

	//--------------------ピクセルシェーダの作成--------------------//
	auto psByteCode = LoadBinaryData(L"PixelShader.cso");
	hr = g_pd3dDevice->CreatePixelShader(
		&psByteCode->front(),
		psByteCode->size(),
		0,
		&g_pPixelShader);
	if (FAILED(hr)) {
		MessageBox(0, L"CreatePixelShader Failed!", 0, 0);
	}

	//--------------------インプットレイアウトを生成する--------------------//
	/// 頂点は...
	/// 座標　法線ベクトル　UV座標　接線ベクトル　バイノーマル　スキンウェイト　...色々持ってる

	/// POSSOPN	：どれに対応してるのか
	/// DXGI...	：RGBそれぞれ4バイト分のデータのfloat
	D3D11_INPUT_ELEMENT_DESC inputElements[] = { { "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 } };
	g_pd3dDevice->CreateInputLayout(
		inputElements, ARRAYSIZE(inputElements),
		&vsByteCode->front(), vsByteCode->size(),
		&g_pInputLayout);
	if (FAILED(hr)) {
		MessageBox(0, L"CreateInputLayout Failed!", 0, 0);
	}
	///ARRAYSIZEは配列の要素数を返してくれるぞ///

	//--------------------頂点バッファの生成--------------------//
	std::vector<XMFLOAT3> BasePos = {
		{-0.5f,  0.5f,  -0.5f},  //0
		{ 0.5f,  0.5f,  -0.5f},  //1
		{-0.5f, -0.5f,  -0.5f},  //2
		{ 0.5f, -0.5f,  -0.5f},  //3

		{+0.5f, +0.5f,  +0.5f},  //4
		{-0.5f, +0.5f,  +0.5f},  //5
		{+0.5f, -0.5f,  +0.5f},  //6
		{-0.5f, -0.5f,  +0.5f}   //7
	};

	std::vector<XMFLOAT3>baseNrm = {  //法線(Normal)
		{0.0f,0.0f,-1.0f}, //手前用
		{+1.0f,0.0f,0.0f},  //右側用
		{-1.0f,0.0f,0.0f}, //左側用
		{0.0f,0.0f,+1.0f},  //奥側用
		{0.0f,+1.0f,0.0f},  //上側用
		{0.0f,-1.0f,1.0f},  //下側用

	};
	// 頂点バッファを生成する
	Vertex vertices[] = {
		// 手前のポリゴン
		{BasePos[0],baseNrm[0]}, // 0
		{BasePos[1],baseNrm[0]}, // 1
		{BasePos[2],baseNrm[0]}, // 2
		{BasePos[3],baseNrm[0]}, // 3

		// 向かって右側面
		{BasePos[1],baseNrm[1]}, // 4
		{BasePos[4],baseNrm[1]}, // 5
		{BasePos[3],baseNrm[1]}, // 6
		{BasePos[5],baseNrm[1]}, // 7

		// 向かって左側面
		{BasePos[5],baseNrm[2]}, // 8
		{BasePos[0],baseNrm[2]}, // 9
		{BasePos[7],baseNrm[2]}, //10
		{BasePos[2],baseNrm[2]}, //11

		// 向かって奥面
		{BasePos[4],baseNrm[3]}, //12
		{BasePos[5],baseNrm[3]}, //13
		{BasePos[6],baseNrm[3]}, //14
		{BasePos[7],baseNrm[3]}, //15

		// 上面
		{BasePos[5],baseNrm[4]}, //16
		{BasePos[4],baseNrm[4]}, //17
		{BasePos[0],baseNrm[4]}, //18
		{BasePos[1],baseNrm[4]}, //19

		// 下面
		{BasePos[2],baseNrm[5]},  //20
		{BasePos[3],baseNrm[5]},  //21
		{BasePos[7],baseNrm[5]},  //22
		{BasePos[6],baseNrm[5]},  //23
	};
	//--------------------頂点バッファの作成--------------------//
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.ByteWidth = sizeof(vertices);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vbSubResource = {};
	vbSubResource.pSysMem = vertices;
	hr = g_pd3dDevice->CreateBuffer(&vbDesc,&vbSubResource,&g_pVertexBuffer);
	if (FAILED(hr)) {
		MessageBox(0, L"CreateBuffer(g_pVertexBuffer) Failed!", 0, 0);
	}

	//--------------------インデックスバッファの作成--------------------//

	//手打ち
	//uint16_t indices[] = {
	//	0,1,2,
	//	2,1,3,
	//	4,5,6,
	//	6,5,7,
	//	8,9,10,
	//	10,9,11,
	//	12,13,14,
	//	14,13,15,
	//	16,17,18,
	//	18,17,19,
	//	20,21,22,
	//	22,21,23
	//};

	std::vector<uint16_t> indices;
	uint16_t indicesBase[] = { 0,1,2,2,1,3 };
	for (int i = 0; i < 6;i++) {
		for (int j = 0; j < 6; j++) {
			indices.push_back(indicesBase[j] + (i*4));
		}
	}
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.ByteWidth = indices.size() * sizeof(uint16_t);
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA ibSubResource = {};
	ibSubResource.pSysMem = &indices.front();
	hr = g_pd3dDevice->CreateBuffer(&ibDesc, &ibSubResource, &g_pIndexBuffer);
	if (FAILED(hr)) {
		MessageBox(0, L"CreateBuffer(g_pIndexBuffer) Failed!", 0, 0);
	}

	//--------------------ビューポートを設定する--------------------//
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = 640;
	viewport.Height = 480;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0f;
	g_pImmediateContext->RSSetViewports(1,&viewport);

	//--------------------コンスタントバッファの生成--------------------//
	ConstantBuffer cb;

	cb.world = DirectX::XMMatrixIdentity();
	//auto t  = DirectX::XMMatrixTranslation(1.0f,0.0f,0.0f);
	//auto rx = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(45.0f));
	//auto ry = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(45.0f));
	//auto rz = DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(45.0f));
	//auto s = DirectX::XMMatrixScaling(2.0f,2.0f,2.0f);
	//cb.world =  s * rz * t;//SRT(Scale Rotation Translation)

	DirectX::XMVECTOR eye = { -4.0f,3.0f,+3.0f };
	DirectX::XMVECTOR at =  { 0.0f,0.0f, 0.0f };
	DirectX::XMVECTOR up =  { 0.0f,1.0f, 0.0f };
	cb.view = DirectX::XMMatrixLookAtLH(eye,at,up);

	float fovAngleY = DirectX::XMConvertToRadians(45.0f);
	float aspectRatio = 640.0f / 480.0f;
	float nearZ = 0.3f,farZ = 1000.0f;
	cb.projection = DirectX::XMMatrixPerspectiveFovLH(fovAngleY,aspectRatio,nearZ,farZ);
	//行列を転置(Transpose)させる...これは行優先の問題
	//行優先(DX)を列優先(Shader)にする
	cb.world = DirectX::XMMatrixTranspose(cb.world);
	cb.view = DirectX::XMMatrixTranspose(cb.view);
	cb.projection = DirectX::XMMatrixTranspose(cb.projection);
	//LH,RHは右手座標系か左手座標系か…
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.ByteWidth = sizeof(cb);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	D3D11_SUBRESOURCE_DATA cbSubResource = {};
	cbSubResource.pSysMem = &cb;
	hr = g_pd3dDevice->CreateBuffer(&cbDesc, &cbSubResource, &g_pConstantBuffer);
	if (FAILED(hr)) {
		MessageBox(0, L"CreateBuffer(g_pConstantBuffer) Failed!", 0, 0);
	}



	//--------------------以下メッセージループ--------------------//
	/// GetMessage(LPMSG lpMsg , HWND hWnd , UINT wMsgFilterMin , UINT wMsgFilterMax);
	/// LPMSG			:MSG構造体のポインタを渡す
	/// HWND			:メッセージを受け取るウィンドウハンドルを渡す
	/// wMsgFilterMin	:受け取るメッセージの最小値
	/// wMsgFilterMax	:受け取るメッセージの最大値
	/// ※フィルタリングしないなら0でおｋ
	///	[返り値]
	/// 通常[TRUE]か[0]以外の数値を返す。WM_QUITを受け取ると[FALSE]、エラーが発生すると[-1]を返す。
	/// [小話]
	/// そもそもの話…
	/// while (GetMessage(&msg, hwnd, 0, 0)...は絶対書いてはいけない
	/// -1がエラー時に帰ってくるのでその時にウィンドウが消えなくなってしまう。
	/// -1を検知して強制終了などを考えないといけない
	//メッセージ構造体(メッセージが格納される)
	MSG msg;
	//背景色
	float color[] = { 0.098f,0.098f,0.439f,1.0f };
	//メッセージチェック
	while (isRunning(&msg))
	{
		//更新処理
		static float radian = 0.0f;
		auto ry = DirectX::XMMatrixRotationX(radian);
		cb.world = DirectX::XMMatrixTranspose(ry);
		g_pImmediateContext->UpdateSubresource(g_pConstantBuffer,0,0,&cb,0,0);
		radian += DirectX::XMConvertToRadians(1.0f);

		g_pImmediateContext->ClearRenderTargetView(g_renderTargetView, color);
		//コンテキストにレンダーターゲットを設定する
		g_pImmediateContext->OMSetRenderTargets(1,&g_renderTargetView,nullptr);
		//描画処理
		UINT stride = sizeof(Vertex);//頂点一個のサイズ
		UINT offset = 0;
		g_pImmediateContext->IASetVertexBuffers(0,1,&g_pVertexBuffer,&stride,&offset);
		g_pImmediateContext->IASetInputLayout(g_pInputLayout);
		//D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP	頂点を共有する(複雑なポリゴンは無理)※共有できる頂点があることが前提
		//D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST		頂点配列をそのまま
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	//頂点データの扱い(データを使って何を表示するのか)

		g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);	//	インデックスバッファ使用時に必須
		g_pImmediateContext->VSSetConstantBuffers(0,1,&g_pConstantBuffer);//(最初,何番目までか,バッファ)
		//DXGI_FORMAT_R16_UINT R16...頂点配列の型。16bitの型なのか32bitの型なのか
		g_pImmediateContext->VSSetShader(g_pVertexShader,0,0);
		g_pImmediateContext->PSSetShader(g_pPixelShader, 0,0);

		//高速化するならマテリアルで分けてDrawを呼ぶ回数を減らすといい…

		//g_pImmediateContext->Draw(ARRAYSIZE(vertices),0);			//頂点数と何番目から描画するか
		g_pImmediateContext->DrawIndexed(indices.size(),0,0);	//インデックスバッファ時の描画

		g_pSwapChain->Present(1,0);			//バックバッファに描画された内容をフロントバッファに

	}

	//使い終わったので解放
	RELEASE(g_renderTargetView);
	RELEASE(g_pSwapChain);
	RELEASE(g_pImmediateContext);
	RELEASE(g_pd3dDevice);


	return 0;
}

///-------------------------------- <summary>
/// ウィンドウプロシージャ
/// メッセージ処理班
/// LRESULT		:コールバック関数やウィンドウプロシージャから返る32bit値
/// CALLBACK	:呼び出し規約
/// [引数]
/// HWND		:メッセージが発生したウィンドウのハンドル
/// UINT		:メッセージ(メッセージを確認したいならココ)
/// WPARAM		:メッセージの付加情報
/// LPARAM		:メッセージの付加情報
/// </summary>-------------------------------
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
		//Windowを閉じる時のメッセージ
	case WM_DESTROY:
		//終了処理
		PostQuitMessage(0);
		break;
		//ESCキー押された時
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

///--------------------------------------<summary>
/// バイナリデータのサイズを返す
/// --[引数]--------------------------------------
/// <param name="filename">ファイルの名前</param>
/// </summary>------------------------------------
std::shared_ptr<std::vector<char>> LoadBinaryData(const std::wstring &filename) {
	using namespace std;
	shared_ptr<vector<char>> result;

	ifstream ifs(filename, ios::in | ios::binary);
	if (ifs) {
		// ファイルのサイズを調べる
		ifs.seekg(0, ios::end);
		size_t size = (size_t)ifs.tellg();
		ifs.seekg(0, ios::beg);

		// メモリ確保
		result = make_shared<vector<char>>();
		result->resize(size);

		// 読み込み
		ifs.read(&result->front(), size);

		//閉じる
		ifs.close();
	}

	return result;
}
