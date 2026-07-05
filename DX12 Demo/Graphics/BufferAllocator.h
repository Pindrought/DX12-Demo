#pragma once
#include "pch.h"
#include "BufferAllocation.h"

class BufferAllocator
{
public:
    void Initialize(u64 size,
                    D3D12_RESOURCE_FLAGS flags,
                    D3D12_RESOURCE_STATES initialState);

    BufferAllocation Allocate(u64 size, u64 alignment);

    void Free(const BufferAllocation& allocation);

    ID3D12Resource* GetResource() const;

private:

    struct FreeBlock
    {
        UINT64 Offset;
        UINT64 Size;
    };

    ComPtr<ID3D12Resource> m_Resource;

    std::list<FreeBlock> m_FreeBlocks;
};