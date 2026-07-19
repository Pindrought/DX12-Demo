#pragma once
#include "pch.h"
#include "BufferAllocation.h"

class Mesh
{
public:
	void BuildBufferAllocations();

	vector<XMFLOAT3> Positions;
	vector<XMFLOAT4> Colors;
	vector<XMFLOAT2> TexCoords;

	BufferAllocation PositionsBufferAllocation;
	BufferAllocation ColorsBufferAllocation;
	BufferAllocation TexCoordsBufferAllocation;

	D3D12_VERTEX_BUFFER_VIEW PositionBufferView;
	D3D12_VERTEX_BUFFER_VIEW ColorBufferView;
	D3D12_VERTEX_BUFFER_VIEW TexCoordBufferView;

	bool IsGPUDataUploaded = false;
	bool IsGPUDataAllocated = false;
	u64 UploadFenceValue = UINT64_MAX;
};