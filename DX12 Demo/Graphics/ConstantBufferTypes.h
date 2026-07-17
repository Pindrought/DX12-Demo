#pragma once
#include "pch.h"

struct PerObjectConstantBuffer
{
    BOOL HasColoredVertices = TRUE;
    float padding[63]; // Padding so the constant buffer is 256-byte aligned.
};
static_assert((sizeof(PerObjectConstantBuffer) % 256) == 0, "PerObjectConstantBuffer must be 256-byte aligned");