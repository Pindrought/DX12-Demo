struct PerObjectData
{
	bool HasColoredVertices;
	bool HasTexCoords;
};

cbuffer _PerObjectDataCB : register(b0, space0)
{
	PerObjectData PerObjectDataCB;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texCoord : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
	if (PerObjectDataCB.HasTexCoords == true)
	{
		return g_texture.Sample(g_sampler, input.texCoord);
	}
	else
	{
		if (PerObjectDataCB.HasColoredVertices == true)
		{
			return input.color;
		}
		else
		{
			return float4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
}