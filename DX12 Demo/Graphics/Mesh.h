#pragma once
#include "pch.h"

class Mesh
{
public:
	vector<XMFLOAT3> Positions;
	vector<XMFLOAT4> Colors;
	D3D12_VERTEX_BUFFER_VIEW PositionBufferView;
	D3D12_VERTEX_BUFFER_VIEW ColorBufferView;
};