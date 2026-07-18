#pragma once
#include "pch.h"

class CommandList
{
public:
	CommandList(D3D12_COMMAND_LIST_TYPE type);
	void Reset();
	void Close();
	void ClearRenderTargetTexture(ID3D12Resource* renderTarget, const FLOAT clearColor[4]);
	ID3D12GraphicsCommandList7* GetD3D12CommandList() const;
	void TrackObject(ComPtr<ID3D12Object> object);
	void ReleaseTrackedObjects();
	D3D12_COMMAND_LIST_TYPE m_CommandListType;
	ComPtr<ID3D12GraphicsCommandList7> m_CommandList;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	std::vector<ComPtr<ID3D12Object>> m_TrackedObjects; //Objects that we need to keep alive until the GPU is done with them. We will release them once the GPU is done with them.
	u32 m_RequiredTransferFenceValue = 0; //The fence value that must be reached before this command list can be executed. This is used to ensure that any resources that are being uploaded to the GPU are done before this command list is executed.
};