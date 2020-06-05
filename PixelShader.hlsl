Texture2D tex;		//�A�v��������n�����e�N�X�`��
SamplerState sam;	//�e�N�X�`���̈�����
struct VSOut {
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float2 tex : TEXCOORD;
};
float4 main(VSOut vsout) : SV_TARGET
{
	//return float4(1.0f, 0.0f, 0.0f, 1.0f);
	//return vsout.color;
	return tex.Sample(sam,vsout.tex) * vsout.color;
}