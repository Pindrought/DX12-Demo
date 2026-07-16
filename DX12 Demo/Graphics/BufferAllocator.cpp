#include "pch.h"
#include "BufferAllocator.h"
#include "Graphics.h"

void BufferAllocator::Initialize(u64 size,
                                 D3D12_RESOURCE_FLAGS flags,
                                 D3D12_RESOURCE_STATES initialState)
{
    if (m_FreeBlocks.size() != 0 || m_Resource != nullptr)
    {
        DBG_LOG("BufferAllocator::Initialize() called for buffer that has already been initialized.");
        throw std::exception("BufferAllocator::Initialize() called for buffer that has already been initialized.");
    }
    auto device = Graphics::GetDevice();
    auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    auto desc = CD3DX12_RESOURCE_DESC::Buffer(size, flags);

    ThrowIfFailed(device->CreateCommittedResource(&heapProps,
                                                  D3D12_HEAP_FLAG_NONE,
                                                  &desc,
                                                  initialState,
                                                  nullptr,
                                                  IID_PPV_ARGS(&m_Resource)));

    m_FreeBlocks.push_back({ 0, size });
}

BufferAllocation BufferAllocator::Allocate(u64 size,
                                           u64 alignment)
{
    for (auto it = m_FreeBlocks.begin(); it != m_FreeBlocks.end(); ++it)
    {
        UINT64 alignedOffset = Align(it->Offset, alignment);

        UINT64 padding = alignedOffset - it->Offset;

        if (padding + size > it->Size)
            continue;

        BufferAllocation alloc;
        alloc.AllocationId = m_NextAllocationId++;
        alloc.Offset = alignedOffset;
        alloc.Size = size;

        UINT64 consumed =
            padding + size;

        it->Offset += consumed;
        it->Size -= consumed;

        if (it->Size == 0)
            m_FreeBlocks.erase(it);

        return alloc;
    }

    return {};
}

void BufferAllocator::Free(const BufferAllocation& allocation)
{
    m_FreeBlocks.push_back(
        {
            allocation.Offset,
            allocation.Size
        });

    //Sort blocks for combining adjacent blocks
    m_FreeBlocks.sort(
        [](auto& a, auto& b)
        {
            return a.Offset < b.Offset;
        });

    for (auto it = m_FreeBlocks.begin();
         it != m_FreeBlocks.end();)
    {
        auto next = std::next(it);

        if (next == m_FreeBlocks.end())
            break;

        //Combine adjacent blocks
        if (it->Offset + it->Size == next->Offset)
        {
            it->Size += next->Size;
            m_FreeBlocks.erase(next);
        }
        else
        {
            ++it;
        }
    }
}

ID3D12Resource* BufferAllocator::GetResource() const
{
    return m_Resource.Get();
}