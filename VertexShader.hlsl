//アプリ側から渡されるデータ群(グローバル宣言扱い)
cbuffer C0 {
	float4x4 world;		//ワールド行列(三次元上のモデルの位置・回転・拡大率)
	float4x4 view;		//ビュー行列(三次元上のカメラの位置・回転)
	float4x4 projection;//射影行列(プロジェクション行列：カメラの画角・アスペクト比(画面比率)・ニアとファー)
};

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	//float4 floatが四つ繋がってるやつ(HLSL標準)
	float4x4 wvp = mul(mul(world,view),projection); //wvp = world * view * projection;
	pos = mul(pos,wvp);								//pos = pos*wvp;
	return pos;
}