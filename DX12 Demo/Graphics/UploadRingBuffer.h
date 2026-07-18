#include "pch.h"
#include "ConstantBufferAllocation.h"

struct SubmissionData
{
    bool IsEmpty() const
    {
        return Start == End;
	}
    u64 Start = 0;
    u64 End = 0;
	u64 FenceValue = UINT64_MAX;
};

class UploadRingBuffer
{
public:
	void Initialize(u64 size);
    ConstantBufferAllocation Allocate(u64 size, u64 alignment = 256u);
    void BeginSubmission();
    void EndSubmission(u64 fenceValue);
	bool CanAllocate(u64 start, u64 size) const;
	void ReclaimCompletedSubmissions(u64 completedFenceValue);
private:
    void NewSubmission();
    ComPtr<ID3D12Resource> m_Buffer;

    uint8_t* m_CPUBase;
    D3D12_GPU_VIRTUAL_ADDRESS m_GPUBase;

    u64 m_Size;
    u64 m_Head;
    u64 m_Tail;

	deque<SubmissionData> m_SubmissionsInFlight;
    vector<SubmissionData> m_SubmissionsInConstruction;
	SubmissionData* m_CurrentSubmission = nullptr;
}; 