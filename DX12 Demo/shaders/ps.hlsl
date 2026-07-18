struct PerObjectData
{
	bool HasColoredVertices;
};

cbuffer _PerObjectDataCB : register(b0, space0)
{
	PerObjectData PerObjectDataCB;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

float4 main(PSInput input) : SV_TARGET
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