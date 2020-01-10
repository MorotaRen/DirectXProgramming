//�\���݂̂����Ȃ���
cbuffer C0 {
	float4x4 world;		//���[���h�s��
	float4x4 view;		//�r���[�s��
	float4x4 projection;//�ˉe�s��(�v���W�F�N�V�����s��)
};

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	//float4 float���l�q�����Ă���(HLSL�W��)
	float4x4 wvp = mul(mul(world,view),projection); //wvp = world * view * projection;
	pos = mul(pos,wvp);								//pos = pos*wvp;
	return pos;
}