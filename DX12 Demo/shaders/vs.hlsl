struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texCoord : TEXCOORD;
};

PSInput main(float4 position : POSITION, float4 color : COLOR, float2 texCoord : TEXCOORD)
{
	PSInput result;

	result.position = position;
	result.color = color;
	result.texCoord = texCoord;

	return result;
}