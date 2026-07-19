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

	D3D12MA::ALLOCATION_DESC allocationDesc = {};
	allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height);

	auto allocator = Graphics::GetInstance()->GetMemoryAllocator(); // You need to add this to Graphics
	ThrowIfFailed(allocator->CreateResource(
		&allocationDesc,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		&Allocation,
		IID_PPV_ARGS(&Resource)));
}
