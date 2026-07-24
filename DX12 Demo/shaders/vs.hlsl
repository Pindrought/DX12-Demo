struct PassData
{
	row_major float4x4 View;
	row_major float4x4 Projection;
	row_major float4x4 ViewProjection;
	float3 CameraPosition;
};

cbuffer _PassData : register(b0, space0)
{
	PassData PassDataCB;
};

struct PerObjectData
{
	row_major float4x4 WorldMatrix;
	bool HasColoredVertices;
	bool HasTexCoords;
	uint TextureId;
};

cbuffer _PerObjectDataCB : register(b1, space0)
{
	PerObjectData PerObjectDataCB;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texCoord : TEXCOORD;
};

PSInput main(float3 position : POSITION, float4 color : COLOR, float2 texCoord : TEXCOORD)
{
	PSInput result;
	float4x4 wvp = mul(PerObjectDataCB.WorldMatrix, PassDataCB.ViewProjection);
	result.position = mul(float4(position, 1.0f), wvp);
	result.color = color;
	result.texCoord = texCoord;

	return result;
}