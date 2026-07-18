#include "pch.h"
#include "UploadRingBuffer.h"
#include "Graphics.h"
#include "Profiler.h"

void UploadRingBuffer::Initialize(u64 size)
{
    m_Size = size;
    m_Head = 0;
    m_Tail = 0;
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
    PROFILE_FUNCTION();

    size = Align(size, alignment);

    u64 allocationStart = Align(m_Head, alignment);

    // Try normal allocation
    if (!CanAllocate(allocationStart, size))
    {
        // Try wrapping
        allocationStart = 0;

        assert(CanAllocate(allocationStart, size) && "UploadRingBuffer exhausted. Need to wait for GPU fence.");

        // We wrapped, so this is a new range in this submission
        if (!m_CurrentSubmission->IsEmpty())
        {
            NewSubmission();
        }

        m_CurrentSubmission->Start = 0;
        m_CurrentSubmission->End = 0;
    }

    ConstantBufferAllocation allocation = {};
    allocation.Offset = allocationStart;
    allocation.CPUAddress = m_CPUBase + allocationStart;
    allocation.GPUAddress = m_GPUBase + allocationStart;
    allocation.Size = size;

    m_Head = allocationStart + size;
    m_CurrentSubmission->End = m_Head;

    return allocation;
}

void UploadRingBuffer::BeginSubmission()
{
    PROFILE_FUNCTION();
	assert(m_SubmissionsInConstruction.empty() == true); //It should be empty as we should not have any submissions in construction at the start of a submission.
	assert(m_CurrentSubmission == nullptr); //We should not have a current submission in progress when starting a new submission.
    NewSubmission();
	m_CurrentSubmission->Start = m_Head;
    m_CurrentSubmission->End = m_Head;
}

void UploadRingBuffer::EndSubmission(u64 fenceValue)
{
    PROFILE_FUNCTION();
	assert(m_CurrentSubmission != nullptr && "EndSubmission called without a corresponding BeginSubmission.");
    for(auto& submission : m_SubmissionsInConstruction)
    {
        submission.FenceValue = fenceValue;
		m_SubmissionsInFlight.push_back(submission);
	}
    m_CurrentSubmission = nullptr;
	m_SubmissionsInConstruction.clear();
}

bool UploadRingBuffer::CanAllocate(u64 start, u64 size) const
{
    PROFILE_FUNCTION();

    u64 end = start + size;

    if (m_Head >= m_Tail)
    {
        // free space is:
        // [head, size) + [0, tail)

        if (start >= m_Head)
            return end <= m_Size;

        return end <= m_Tail;
    }
    else
    {
        // free space is:
        // [head, tail)

        return end <= m_Tail;
    }
}

void UploadRingBuffer::ReclaimCompletedSubmissions(u64 completedFenceValue)
{
    PROFILE_FUNCTION();
    while (!m_SubmissionsInFlight.empty())
    {
        auto& submission = m_SubmissionsInFlight.front();

        if (submission.FenceValue <= completedFenceValue)
        {
            m_Tail = submission.End;
            m_SubmissionsInFlight.pop_front();
        }
        else
        {
            break;
        }
    }
}

void UploadRingBuffer::NewSubmission()
{
    PROFILE_FUNCTION();

    m_SubmissionsInConstruction.push_back(SubmissionData());
    m_CurrentSubmission = &m_SubmissionsInConstruction.back();
}
