#include "pch.h"
#include "Texture.h"
#include "Graphics.h"
void Texture::Initialize(vector<u8> textureData, 
						 UINT width, UINT height, DXGI_FORMAT format)
{
	auto device = Graphics::GetDevice();
	Format = format;
	Width = width;
	Height = height;
	Data = textureData;
	CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height);
	ThrowIfFailed(device->CreateCommittedResource(
						&defaultHeap,
						D3D12_HEAP_FLAG_NONE,
						&desc,
						D3D12_RESOURCE_STATE_COPY_DEST,
						nullptr,
						IID_PPV_ARGS(&Resource)));
}
