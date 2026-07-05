#include "pch.h"
#include "CommandList.h"
#include "Graphics.h"

CommandList::CommandList(D3D12_COMMAND_LIST_TYPE type)
    :m_CommandListType(type)
{
    auto pDevice = Graphics::GetDevice();

    ThrowIfFailed(pDevice->CreateCommandAllocator(m_CommandListType, 
                                                  IID_PPV_ARGS(&m_CommandAllocator)));

    ThrowIfFailed(pDevice->CreateCommandList(0, 
                                             m_CommandListType, 
                                             m_CommandAllocator.Get(), 
                                             nullptr,
                                             IID_PPV_ARGS(&m_CommandList)));

	m_CommandList->Close(); // Command lists are created in the recording state. Since there is nothing to record right now, close it.

    //TODO: Figure out what this is
    /*m_UploadBuffer = std::make_unique<MakeUploadBuffer>(device);

    m_ResourceStateTracker = std::make_unique<ResourceStateTracker>();

    for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        m_DynamicDescriptorHeap[i] =
            std::make_unique<DynamicDescriptorHeap>(device, static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
        m_DescriptorHeaps[i] = nullptr;
    }*/

}

void CommandList::Reset()
{
    ThrowIfFailed(m_CommandAllocator->Reset());
    ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), nullptr));

    ReleaseTrackedObjects();


    //Todo: come back to this when I need it
    //m_ResourceStateTracker->Reset();
    //m_UploadBuffer->Reset();

    //ReleaseTrackedObjects();

    //for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    //{
    //    m_DynamicDescriptorHeap[i]->Reset();
    //    m_DescriptorHeaps[i] = nullptr;
    //}

    //m_RootSignature = nullptr;
    //m_PipelineState = nullptr;
    //m_ComputeCommandList = nullptr;
}

void CommandList::Close()
{
    ThrowIfFailed(m_CommandList->Close());
}

void CommandList::ClearRenderTargetTexture(ID3D12Resource* renderTarget, const FLOAT clearColor[4])
{
}

ID3D12GraphicsCommandList7* CommandList::GetD3D12CommandList() const
{
    return m_CommandList.Get();
}

void CommandList::TrackObject(ComPtr<ID3D12Object> object)
{
	m_TrackedObjects.push_back(object);
}

void CommandList::ReleaseTrackedObjects()
{
    m_TrackedObjects.clear();
}
