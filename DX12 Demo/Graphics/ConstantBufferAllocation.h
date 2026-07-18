#pragma once
#include "pch.h"

struct ConstantBufferAllocation
{
    D3D12_GPU_VIRTUAL_ADDRESS GPUAddress;
    void* CPUAddress;
    size_t Offset;
    size_t Size;
};