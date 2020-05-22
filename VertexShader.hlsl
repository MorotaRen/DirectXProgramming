//アプリ側から渡されるデータ群(グローバル宣言扱い)
//コンスタントバッファ…cbuffer
cbuffer C0 {
	float4x4 world;		//ワールド行列(三次元上のモデルの位置・回転・拡大率)
	float4x4 view;		//ビュー行列(三次元上のカメラの位置・回転)
	float4x4 projection;//射影行列(プロジェクション行列：カメラの画角・アスペクト比(画面比率)・ニアとファー)
};
struct VSIn {
	float3 pos : POSITION;
	float3 normal : NORMAL;
};
float4 main(VSIn vsin) : SV_POSITION
{

	float3 vLight = {0.0f,-1.0f,0.0f};

//wvp = world * view * projection;
float4x4 wvp = mul(mul(world,view),projection);
float4 pos = mul(float4(vsin.pos,1.0f), wvp);

//陰影の計算
float3 L = normalize(-vLight);
float3 N = vsin.normal;
float4 color = float4(1, 1, 1, 1) * dot(L, N);
color.a = 1.0f;
return pos;
}
