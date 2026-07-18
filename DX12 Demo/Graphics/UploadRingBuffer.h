#include "pch.h"
#include "ConstantBufferAllocation.h"

class UploadRingBuffer
{
public:
	void Initialize(u64 size);
    ConstantBufferAllocation Allocate(u64 size, u64 alignment);

private:

    ComPtr<ID3D12Resource> m_Buffer;

    uint8_t* m_CPUBase;
    D3D12_GPU_VIRTUAL_ADDRESS m_GPUBase;

    size_t m_Size;
    size_t m_Head;
};