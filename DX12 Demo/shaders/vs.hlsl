cbuffer SceneConstantBuffer : register(b0, space0)
{
	float4 offset;
	float4 padding[15];
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput main(float4 position : POSITION, float4 color : COLOR)
{
	PSInput result;

	result.position = position;
	result.color = color;

	return result;
}