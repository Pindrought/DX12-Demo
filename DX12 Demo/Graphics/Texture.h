#pragma once
#include "pch.h"
#include "BufferAllocation.h"

class Texture
{
public:
	void Initialize(vector<u8> textureData, UINT width, UINT height, DXGI_FORMAT format);
	ComPtr<ID3D12Resource> Resource;
	DXGI_FORMAT Format;
	u32 Width = 0;
	u32 Height = 0;
	vector<u8> Data;
	bool IsGPUDataUploaded = false;
};