#include <windows.h>

//----------------------------------Direct3D���g�����߂̃��C�u����----------------------------------//
#include <d3d11_4.h>
#pragma comment(lib,"d3d11.lib")

//-------------------------------------------���C�u����--------------------------------------------//
#include <string>
#include <vector>
#include <fstream>
#include <DirectXMath.h>
using namespace DirectX;

//----------------------------------��ɐ錾------------------------------------//
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
std::shared_ptr<std::vector<char>> LoadBinaryData(const std::wstring &filename);

//----------------------------------����}�N��----------------------------------//
//"\"����ꂽ����s�ł��邪��{��s
#define RELEASE(x) \
do{\
	if (x) {\
			(x)->Release(); \
			(x) = nullptr; \
	}\
}while (0);\


//----------------------------------�O���[�o���ϐ�----------------------------------//
HWND						g_hwnd;									//�E�B���h�E�n���h��
IDXGISwapChain				*g_pSwapChain = nullptr;				//�X���b�v�`�F�C��
ID3D11Device				*g_pd3dDevice = nullptr;				//�f�o�C�X
ID3D11DeviceContext			*g_pImmediateContext = nullptr;			//�R���e�L�X�g
ID3D11RenderTargetView		*g_renderTargetView = nullptr;			//�����_�[�^�[�Q�b�g�r���[
ID3D11VertexShader			*g_pVertexShader = nullptr;				//�o�[�e�b�N�X�V�F�[�_�[
ID3D11PixelShader			*g_pPixelShader = nullptr;				//�s�N�Z���V�F�[�_�[
ID3D11InputLayout			*g_pInputLayout = nullptr;				//�C���v�b�g���C�A�E�g
ID3D11Buffer				*g_pVertexBuffer = nullptr;				//�o�[�e�b�N�X�o�b�t�@
ID3D11Buffer				*g_pIndexBuffer = nullptr;				//�C���f�b�N�X�o�b�t�@
ID3D11Buffer				*g_pConstantBuffer = nullptr;			//�R���X�^���g�o�b�t�@

//----------------------------------���_�f�[�^----------------------------------//
struct Vertex
{
	//float x, y, z;
	XMFLOAT3 position;  //���_���W
	XMFLOAT3 normal;    //�@���x�N�g��
};

//----------------�R���X�^���g�o�b�t�@�̌��f�[�^(16�o�C�g��؂�)---------------------//
struct ConstantBuffer
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;

	//16��؂�ɂ��Ȃ��Ƃ����Ȃ��̂�
	//float time;//4byte
	//float dummy[3]//12byte
	//�_�~�[�ǉ����č��v16byte�ɂȂ�悤�ɂ��Ȃ��Ƃ����Ȃ�
};

bool isRunning(MSG* msg) {
	if(PeekMessage(msg, 0, 0, 0, PM_REMOVE)){
		if(msg->message == WM_QUIT) {
			return false;
		}
			//���b�Z�[�W�𑗐M
			DispatchMessage(msg);
	}
	return true;
}

///-------------------------------- <summary>
/// Main�֐��̑���
/// WINAPI�����邱��
/// WINAPI		:Win32API���Ăяo�����̋K��
/// [����]
/// HINSTANCE	:�C���X�^���X�n���h���A�ꕔ��API�֐����Ăяo�����Ɏg��
/// HINSTANCE	:Win16�̎Y���A���͏��NULL
/// LPSTR		:�R�}���h���C������󂯎��������
/// int			:�����\�����@���w��ł���
/// </summary>-------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	//	[�E�B���h�E�̍���]
	//1	�E�B���h�E�N���X�̍쐬
	//2	��������̂�Windows�ɓo�^
	//3	�E�B���h�E����


	//--------------------�E�B���h�E�N���X�̃f�[�^�쐬--------------------//
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;			//Window�X�^�C��
	wc.lpfnWndProc = WindowProc;				//WindowProcedure
	wc.cbClsExtra = 0;							//EX�̈�A���̂Ƃ���Ӗ��Ȃ�
	wc.cbWndExtra = 0;							//EX�̈�A���̂Ƃ���Ӗ��Ȃ�
	wc.hInstance = GetModuleHandle(NULL);		//�A�v���P�[�V�����C���X�^���X���擾
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);	//Window�A�C�R��
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);	//Window���Ŏg�p�����J�[�\��
	wc.hbrBackground = 0;						//�N���C�A���g�̈�̔w�i
	wc.lpszMenuName = NULL;						//���j���[�l�[��
	wc.lpszClassName = L"Title";				//�E�B���h�E�N���X�̖��O

	//--------------------�E�B���h�E�N���X�̓o�^-------------------------//
	RegisterClass(&wc);
	
	//------------------------�E�B���h�E�̐���--------------------------//
	g_hwnd = CreateWindow(L"Title", L"Title", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 640, 480, 0, 0, wc.hInstance, 0);


	//------DirectX������������------------
	//1.Direct3D�I�u�W�F�N�g�𐶐�����
	//2.�f�o�C�X�R���e�L�X�g�𐶐�����
	//3.�X���b�v�`�F�C���𐶐�����
	//4.�o�b�N�o�b�t�@�Ǝ擾���ă����_�[�^�[�Q�b�g�r���[�𐶐�����
	//--------���\�[�X����-----------------
	//5.���_�V�F�[�_�𐶐�����
	//6.�s�N�Z���V�F�[�_�𐶐�����
	//7.�C���v�b�g���C�A�E�g�𐶐�����
	//8.���_(�o�[�e�b�N�X)�o�b�t�@�𐶐�����
	//9.�C���f�b�N�X�o�b�t�@�𐶐�����
	//10.�R���X�^���g�o�b�t�@�𐶐�����
	//------------�`��--------------------
	//11.��ʂ��N���A
	//12.�o�b�N�o�b�t�@�Ƀ|���S������
	//13.�o�b�N�o�b�t�@�ƃt�����g�o�b�t�@�����ւ���


	/// --------------------------------<summary>
	/// sd.BufferCount							//�o�b�N�o�b�t�@�̐�
	/// sd.BufferDesc.Width						//���𑜓x
	/// sd.BufferDesc.Height					//�c�𑜓x
	/// sd.BufferDesc.Format					//�`���`��
	/// sd.BufferDesc.RefreshRate.Numerator		//���t���b�V�����[�g�̕��q
	/// sd.BufferDesc.RefreshRate.Denominator	//���t���b�V�����[�g�̕���
	/// sd.BufferUsage							//�������邽�߂̂��̂Ȃ̂�
	/// sd.OutputWindow							//�E�B���h�E�n���h��
	/// sd.SampleDesc.Count						//
	/// sd.SampleDesc.Quality					//MSAA�ݒ�
	/// sd.Windowed								//�E�B���h�E���[�h��
	/// </summary>--------------------------------

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));//0�ŏ�����
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
	/// IDXGIAdapter,				// �ǂ̃r�f�I�A�_�v�^���g�p���邩�H����Ȃ��nullptr�ŁAIDXGIAdapter�̃A�h���X��n��.
	///	D3D_DRIVER_TYPE,			// �h���C�o�̃^�C�v��n���B����ȊO�͊�{�I�Ƀ\�t�g�E�F�A�����ŁA�ǂ����Ă��Ƃ�������f�o�O�p�ɗp����ׂ�.
	///	HMODULE,					// ��L��D3D_DRIVER_TYPE_SOFTWARE�ɐݒ肵���ۂɁA���̏������s��DLL�̃n���h����n���B����ȊO���w�肵�Ă���ۂɂ͕K��nullptr��n��.
	///	UINT,						// ���炩�̃t���O���w�肷��B�ڂ�����D3D11_CREATE_DEVICE�񋓌^�Ō���������.
	///	const D3D_FEATURE_LEVEL,	// ���͂�����D3D_FEATURE_LEVEL�񋓌^�̔z���^����Bnullptr�ɂ��邱�Ƃŏ�Lfeature�Ɠ����̓��e�̔z�񂪎g�p�����.
	///	UINT,						// ��L�����ŁA�����Œ�`�����z���^���Ă����ꍇ�A���̔z��̗v�f���������ɋL�q����.
	///	UINT,						// SDK�̃o�[�W�����B�K�����̒l->D3D11_SDK_VERSION
	///	const DXGI_SWAP_CHAIN_DESC,	// DXGI_SWAP_CHAIN_DESC�\���̂̃A�h���X��ݒ肷��B�����Őݒ肵���\�����ɐݒ肳��Ă���p�����[�^��SwapChain���쐬�����.
	///	IDXGISwapChain,				// �쐬�����������ꍇ�ɁA����SwapChain�̃A�h���X���i�[����|�C���^�ϐ��ւ̃A�h���X�B�����Ŏw�肵���|�C���^�ϐ��o�R��SwapChain�𑀍삷��.
	///	ID3D11Device,				// ��L�Ƃقړ��l�ŁA������ɂ�Device�̃|�C���^�ϐ��̃A�h���X��ݒ肷��.
	///	D3D_FEATURE_LEVEL,			// ���ۂɍ쐬�ɐ�������D3D_FEATURE_LEVEL���i�[���邽�߂�D3D_FEATURE_LEVEL�񋓌^�ϐ��̃A�h���X��ݒ肷��.
	///	ID3D11DeviceContext			// SwapChain��Device�Ɠ��l�ɁA������ɂ�Context�̃|�C���^�ϐ��̃A�h���X��ݒ肷��.
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

	//--------------------�o�b�N�o�b�t�@�擾--------------------//
	ID3D11Texture2D *backbuffer = nullptr;
	/// GetBuffer(UINT,REFID,void**)
	/// UINT	:�o�b�t�@�C���f�b�N�X(��{0)
	/// REFID	:�擾����o�b�t�@�̃C���^�[�t�F�[�XID
	/// void**	:�o�b�t�@�̎擾��
	/// [void**�ɂ���]
	/// �ėp�|�C���^�A�Ȃ�ł����邪�L���X�g�K�{
	hr = g_pSwapChain->GetBuffer(0,__uuidof(ID3D11Texture2D),(void**)&backbuffer);
	if (FAILED(hr)) {
		MessageBox(0, L"GetBuffer Failed!", 0, 0);
	}

	//----------------�����_�[�^�[�Q�b�g�r���[�쐬----------------//
	/// CreateRenderTargetView(ID3D11Resource*,const D3D11_RENDER_TARGET_VIEW_DESC *,ID3D11RenderTargetView**);
	/// ID3D11Resource					:�쐬����o�b�t�@�̃��\�[�X
	/// D3D11_RENDER_TARGET_VIEW_DESC	:�쐬����View�̐ݒ�,nullptr�Ńf�t�H���g�ݒ�
	/// ID3D11RenderTargetView			:�쐬���ꂽ���̂̎��[���鏊�̃|�C���^
	hr = g_pd3dDevice->CreateRenderTargetView(backbuffer,nullptr,&g_renderTargetView);
	if (FAILED(hr)) {
		MessageBox(0, L"CreateRenderTargetView Failed!", 0, 0);
	}

	//�g���I������̂ŉ���ł���
	RELEASE(backbuffer);

	//--------------------���_�V�F�[�_�[�̓ǂݍ��݂Ɛ����쐬--------------------//
	/// CreateVertexShader
	///	const void         *pShaderBytecode		:�R���p�C���ς݃V�F�[�_�̃o�C�i���f�[�^
	///	SIZE_T             BytecodeLength		:�o�C�i���f�[�^�T�C�Y(Byte)
	///	ID3D11ClassLinkage *pClassLinkage		:�N���X�����P�[�W�C���^�[�t�F�C�X�ւ̃|�C���^(NULL�\)
	///	ID3D11VertexShader **ppVertexShader		:�������ꂽ�V�F�[�_�[

	auto vsByteCode = LoadBinaryData(L"VertexShader.cso");
	hr = g_pd3dDevice->CreateVertexShader(
	&vsByteCode->front(),
	vsByteCode->size(),
	0,
	&g_pVertexShader);
	if (FAILED(hr)) {
		MessageBox(0, L"CreateVertexShader Failed!", 0, 0);
	}

	//--------------------�s�N�Z���V�F�[�_�̍쐬--------------------//
	auto psByteCode = LoadBinaryData(L"PixelShader.cso");
	hr = g_pd3dDevice->CreatePixelShader(
		&psByteCode->front(),
		psByteCode->size(),
		0,
		&g_pPixelShader);
	if (FAILED(hr)) {
		MessageBox(0, L"CreatePixelShader Failed!", 0, 0);
	}

	//--------------------�C���v�b�g���C�A�E�g�𐶐�����--------------------//
	/// ���_��...
	/// ���W�@�@���x�N�g���@UV���W�@�ڐ��x�N�g���@�o�C�m�[�}���@�X�L���E�F�C�g�@...�F�X�����Ă�

	/// POSSOPN	�F�ǂ�ɑΉ����Ă�̂�
	/// DXGI...	�FRGB���ꂼ��4�o�C�g���̃f�[�^��float
	D3D11_INPUT_ELEMENT_DESC inputElements[] = { { "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 } };
	g_pd3dDevice->CreateInputLayout(
		inputElements, ARRAYSIZE(inputElements),
		&vsByteCode->front(), vsByteCode->size(),
		&g_pInputLayout);
	if (FAILED(hr)) {
		MessageBox(0, L"CreateInputLayout Failed!", 0, 0);
	}
	///ARRAYSIZE�͔z��̗v�f����Ԃ��Ă���邼///

	//--------------------���_�o�b�t�@�̐���--------------------//
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

	std::vector<XMFLOAT3>baseNrm = {  //�@��(Normal)
		{0.0f,0.0f,-1.0f}, //��O�p
		{+1.0f,0.0f,0.0f},  //�E���p
		{-1.0f,0.0f,0.0f}, //�����p
		{0.0f,0.0f,+1.0f},  //�����p
		{0.0f,+1.0f,0.0f},  //�㑤�p
		{0.0f,-1.0f,1.0f},  //�����p

	};
	// ���_�o�b�t�@�𐶐�����
	Vertex vertices[] = {
		// ��O�̃|���S��
		{BasePos[0],baseNrm[0]}, // 0
		{BasePos[1],baseNrm[0]}, // 1
		{BasePos[2],baseNrm[0]}, // 2
		{BasePos[3],baseNrm[0]}, // 3

		// �������ĉE����
		{BasePos[1],baseNrm[1]}, // 4
		{BasePos[4],baseNrm[1]}, // 5
		{BasePos[3],baseNrm[1]}, // 6
		{BasePos[5],baseNrm[1]}, // 7

		// �������č�����
		{BasePos[5],baseNrm[2]}, // 8
		{BasePos[0],baseNrm[2]}, // 9
		{BasePos[7],baseNrm[2]}, //10
		{BasePos[2],baseNrm[2]}, //11

		// �������ĉ���
		{BasePos[4],baseNrm[3]}, //12
		{BasePos[5],baseNrm[3]}, //13
		{BasePos[6],baseNrm[3]}, //14
		{BasePos[7],baseNrm[3]}, //15

		// ���
		{BasePos[5],baseNrm[4]}, //16
		{BasePos[4],baseNrm[4]}, //17
		{BasePos[0],baseNrm[4]}, //18
		{BasePos[1],baseNrm[4]}, //19

		// ����
		{BasePos[2],baseNrm[5]},  //20
		{BasePos[3],baseNrm[5]},  //21
		{BasePos[7],baseNrm[5]},  //22
		{BasePos[6],baseNrm[5]},  //23
	};
	//--------------------���_�o�b�t�@�̍쐬--------------------//
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.ByteWidth = sizeof(vertices);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vbSubResource = {};
	vbSubResource.pSysMem = vertices;
	hr = g_pd3dDevice->CreateBuffer(&vbDesc,&vbSubResource,&g_pVertexBuffer);
	if (FAILED(hr)) {
		MessageBox(0, L"CreateBuffer(g_pVertexBuffer) Failed!", 0, 0);
	}

	//--------------------�C���f�b�N�X�o�b�t�@�̍쐬--------------------//

	//��ł�
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

	//--------------------�r���[�|�[�g��ݒ肷��--------------------//
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = 640;
	viewport.Height = 480;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0f;
	g_pImmediateContext->RSSetViewports(1,&viewport);

	//--------------------�R���X�^���g�o�b�t�@�̐���--------------------//
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
	//�s���]�u(Transpose)������...����͍s�D��̖��
	//�s�D��(DX)���D��(Shader)�ɂ���
	cb.world = DirectX::XMMatrixTranspose(cb.world);
	cb.view = DirectX::XMMatrixTranspose(cb.view);
	cb.projection = DirectX::XMMatrixTranspose(cb.projection);
	//LH,RH�͉E����W�n��������W�n���c
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.ByteWidth = sizeof(cb);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	D3D11_SUBRESOURCE_DATA cbSubResource = {};
	cbSubResource.pSysMem = &cb;
	hr = g_pd3dDevice->CreateBuffer(&cbDesc, &cbSubResource, &g_pConstantBuffer);
	if (FAILED(hr)) {
		MessageBox(0, L"CreateBuffer(g_pConstantBuffer) Failed!", 0, 0);
	}



	//--------------------�ȉ����b�Z�[�W���[�v--------------------//
	/// GetMessage(LPMSG lpMsg , HWND hWnd , UINT wMsgFilterMin , UINT wMsgFilterMax);
	/// LPMSG			:MSG�\���̂̃|�C���^��n��
	/// HWND			:���b�Z�[�W���󂯎��E�B���h�E�n���h����n��
	/// wMsgFilterMin	:�󂯎�郁�b�Z�[�W�̍ŏ��l
	/// wMsgFilterMax	:�󂯎�郁�b�Z�[�W�̍ő�l
	/// ���t�B���^�����O���Ȃ��Ȃ�0�ł���
	///	[�Ԃ�l]
	/// �ʏ�[TRUE]��[0]�ȊO�̐��l��Ԃ��BWM_QUIT���󂯎���[FALSE]�A�G���[�����������[-1]��Ԃ��B
	/// [���b]
	/// ���������̘b�c
	/// while (GetMessage(&msg, hwnd, 0, 0)...�͐�Ώ����Ă͂����Ȃ�
	/// -1���G���[���ɋA���Ă���̂ł��̎��ɃE�B���h�E�������Ȃ��Ȃ��Ă��܂��B
	/// -1�����m���ċ����I���Ȃǂ��l���Ȃ��Ƃ����Ȃ�
	//���b�Z�[�W�\����(���b�Z�[�W���i�[�����)
	MSG msg;
	//�w�i�F
	float color[] = { 0.098f,0.098f,0.439f,1.0f };
	//���b�Z�[�W�`�F�b�N
	while (isRunning(&msg))
	{
		//�X�V����
		static float radian = 0.0f;
		auto ry = DirectX::XMMatrixRotationX(radian);
		cb.world = DirectX::XMMatrixTranspose(ry);
		g_pImmediateContext->UpdateSubresource(g_pConstantBuffer,0,0,&cb,0,0);
		radian += DirectX::XMConvertToRadians(1.0f);

		g_pImmediateContext->ClearRenderTargetView(g_renderTargetView, color);
		//�R���e�L�X�g�Ƀ����_�[�^�[�Q�b�g��ݒ肷��
		g_pImmediateContext->OMSetRenderTargets(1,&g_renderTargetView,nullptr);
		//�`�揈��
		UINT stride = sizeof(Vertex);//���_��̃T�C�Y
		UINT offset = 0;
		g_pImmediateContext->IASetVertexBuffers(0,1,&g_pVertexBuffer,&stride,&offset);
		g_pImmediateContext->IASetInputLayout(g_pInputLayout);
		//D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP	���_�����L����(���G�ȃ|���S���͖���)�����L�ł��钸�_�����邱�Ƃ��O��
		//D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST		���_�z������̂܂�
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	//���_�f�[�^�̈���(�f�[�^���g���ĉ���\������̂�)

		g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);	//	�C���f�b�N�X�o�b�t�@�g�p���ɕK�{
		g_pImmediateContext->VSSetConstantBuffers(0,1,&g_pConstantBuffer);//(�ŏ�,���Ԗڂ܂ł�,�o�b�t�@)
		//DXGI_FORMAT_R16_UINT R16...���_�z��̌^�B16bit�̌^�Ȃ̂�32bit�̌^�Ȃ̂�
		g_pImmediateContext->VSSetShader(g_pVertexShader,0,0);
		g_pImmediateContext->PSSetShader(g_pPixelShader, 0,0);

		//����������Ȃ�}�e���A���ŕ�����Draw���Ăԉ񐔂����炷�Ƃ����c

		//g_pImmediateContext->Draw(ARRAYSIZE(vertices),0);			//���_���Ɖ��Ԗڂ���`�悷�邩
		g_pImmediateContext->DrawIndexed(indices.size(),0,0);	//�C���f�b�N�X�o�b�t�@���̕`��

		g_pSwapChain->Present(1,0);			//�o�b�N�o�b�t�@�ɕ`�悳�ꂽ���e���t�����g�o�b�t�@��

	}

	//�g���I������̂ŉ��
	RELEASE(g_renderTargetView);
	RELEASE(g_pSwapChain);
	RELEASE(g_pImmediateContext);
	RELEASE(g_pd3dDevice);


	return 0;
}

///-------------------------------- <summary>
/// �E�B���h�E�v���V�[�W��
/// ���b�Z�[�W������
/// LRESULT		:�R�[���o�b�N�֐���E�B���h�E�v���V�[�W������Ԃ�32bit�l
/// CALLBACK	:�Ăяo���K��
/// [����]
/// HWND		:���b�Z�[�W�����������E�B���h�E�̃n���h��
/// UINT		:���b�Z�[�W(���b�Z�[�W���m�F�������Ȃ�R�R)
/// WPARAM		:���b�Z�[�W�̕t�����
/// LPARAM		:���b�Z�[�W�̕t�����
/// </summary>-------------------------------
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
		//Window����鎞�̃��b�Z�[�W
	case WM_DESTROY:
		//�I������
		PostQuitMessage(0);
		break;
		//ESC�L�[�����ꂽ��
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
/// �o�C�i���f�[�^�̃T�C�Y��Ԃ�
/// --[����]--------------------------------------
/// <param name="filename">�t�@�C���̖��O</param>
/// </summary>------------------------------------
std::shared_ptr<std::vector<char>> LoadBinaryData(const std::wstring &filename) {
	using namespace std;
	shared_ptr<vector<char>> result;

	ifstream ifs(filename, ios::in | ios::binary);
	if (ifs) {
		// �t�@�C���̃T�C�Y�𒲂ׂ�
		ifs.seekg(0, ios::end);
		size_t size = (size_t)ifs.tellg();
		ifs.seekg(0, ios::beg);

		// �������m��
		result = make_shared<vector<char>>();
		result->resize(size);

		// �ǂݍ���
		ifs.read(&result->front(), size);

		//����
		ifs.close();
	}

	return result;
}
