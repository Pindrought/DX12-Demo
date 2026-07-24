#pragma once
#include "pch.h"

struct alignas(256) PerPassConstantBufferData
{
    DirectX::XMFLOAT4X4 View;
    DirectX::XMFLOAT4X4 Projection;
    DirectX::XMFLOAT4X4 ViewProjection;
    DirectX::XMFLOAT3 CameraPosition;
};

struct alignas(256) PerObjectConstantBufferData
{
    Matrix WorldMatrix = Matrix::Identity;
    BOOL HasColoredVertices = TRUE;
    BOOL HasTexCoords = FALSE;
    UINT TextureId = 0;
};
static_assert(alignof(PerObjectConstantBufferData) == 256);
static_assert(sizeof(PerObjectConstantBufferData) == 256);