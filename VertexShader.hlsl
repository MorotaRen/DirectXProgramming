//�A�v��������n�����f�[�^�Q(�O���[�o���錾����)
//�R���X�^���g�o�b�t�@�ccbuffer
cbuffer C0 {
	float4x4 world;		//���[���h�s��(�O������̃��f���̈ʒu�E��]�E�g�嗦)
	float4x4 view;		//�r���[�s��(�O������̃J�����̈ʒu�E��])
	float4x4 projection;//�ˉe�s��(�v���W�F�N�V�����s��F�J�����̉�p�E�A�X�y�N�g��(��ʔ䗦)�E�j�A�ƃt�@�[)
	float4 c0_color;//�J���[
};
struct VSIn {
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD;
};
struct VSOut {
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float2 tex : TEXCOORD;
};
VSOut main(VSIn vsin)
{


//wvp = world * view * projection;
float4x4 wvp = mul(mul(world,view),projection);
float4 pos = mul(float4(vsin.pos,1.0f), wvp);

//�A�e�̌v�Z
float3 vLight = { 1.0f,-1.0f,0.0f };
float3 L = normalize(-vLight);
float3 N = mul(vsin.normal,(float3x3)world);
float4 color = c0_color * dot(L, N);
color.a = 1.0f;
VSOut vsout;
vsout.pos = pos;
vsout.color = color;
vsout.tex = vsin.tex;

return vsout;
}
