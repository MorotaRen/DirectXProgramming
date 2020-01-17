//�A�v��������n�����f�[�^�Q(�O���[�o���錾����)
cbuffer C0 {
	float4x4 world;		//���[���h�s��(�O������̃��f���̈ʒu�E��]�E�g�嗦)
	float4x4 view;		//�r���[�s��(�O������̃J�����̈ʒu�E��])
	float4x4 projection;//�ˉe�s��(�v���W�F�N�V�����s��F�J�����̉�p�E�A�X�y�N�g��(��ʔ䗦)�E�j�A�ƃt�@�[)
};

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	//float4 float���l�q�����Ă���(HLSL�W��)
	float4x4 wvp = mul(mul(world,view),projection); //wvp = world * view * projection;
	pos = mul(pos,wvp);								//pos = pos*wvp;
	return pos;
}