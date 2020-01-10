//構造体みたいなもの
cbuffer C0 {
	float4x4 world;		//ワールド行列
	float4x4 view;		//ビュー行列
	float4x4 projection;//射影行列(プロジェクション行列)
};

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	//float4 floatが四つ繋がってるやつ(HLSL標準)
	float4x4 wvp = mul(mul(world,view),projection); //wvp = world * view * projection;
	pos = mul(pos,wvp);								//pos = pos*wvp;
	return pos;
}