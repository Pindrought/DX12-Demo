#include "pch.h"
#include "UploadRingBuffer.h"
#include "Graphics.h"

void UploadRingBuffer::Initialize(u64 size)
{
    m_Size = size;
    m_Head = 0;
    auto device = Graphics::GetDevice();
    auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);
    ThrowIfFailed(device->CreateCommittedResource(&heapProps,
                                                  D3D12_HEAP_FLAG_NONE,
                                                  &desc,
                                                  D3D12_RESOURCE_STATE_GENERIC_READ,
                                                  nullptr,
                                                  IID_PPV_ARGS(&m_Buffer)));
    m_CPUBase = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    ThrowIfFailed(m_Buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_CPUBase)));
	m_GPUBase = m_Buffer->GetGPUVirtualAddress();
}

ConstantBufferAllocation UploadRingBuffer::Allocate(u64 size, u64 alignment)
{
    size = Align(size, 256);

    if (m_Head + size > m_Size)
    {
        m_Head = 0;
    }

	assert((m_Head + size) <= m_Size && "UploadRingBuffer overflow. Increase the buffer size.");

    ConstantBufferAllocation allocation = {};

    allocation.Offset = m_Head;

    allocation.CPUAddress = m_CPUBase + m_Head;

    allocation.GPUAddress = m_GPUBase + m_Head;

    allocation.Size = size;

    m_Head += size;

    return allocation;
}