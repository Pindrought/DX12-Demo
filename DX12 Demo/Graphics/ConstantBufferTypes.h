#pragma once
#include "pch.h"

struct alignas(256) PerObjectConstantBufferData
{
    BOOL HasColoredVertices = TRUE;
};
static_assert(alignof(PerObjectConstantBufferData) == 256);
static_assert(sizeof(PerObjectConstantBufferData) == 256);