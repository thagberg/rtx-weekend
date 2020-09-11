Texture2D tx : register(t0);
SamplerState bilinearSampler : register(s0);

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	return tx.Sample(bilinearSampler, input.Tex);
}