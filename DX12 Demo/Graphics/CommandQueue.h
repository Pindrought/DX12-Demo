#pragma once
#include "pch.h"
#include "CommandList.h"

struct CommandListEntry
{
	u64 FenceValue;
	shared_ptr<CommandList> CommandList;
};;

class CommandQueue
{
public:
	CommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
	ID3D12CommandQueue* GetD3D12CommandQueue() const;
	shared_ptr<CommandList> GetCommandList();
	u64 ExecuteCommandList(shared_ptr<CommandList> commandList);
	u64 ExecuteCommandLists(const vector<shared_ptr<CommandList>>& commandLists);
	bool IsFenceComplete(u64 fenceValue);
	u64 Signal();
	void WaitForFenceValue(u64 fenceValue);
	void RecycleInFlightCommandLists();
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	D3D12_COMMAND_LIST_TYPE m_CommandListType;

	ComPtr<ID3D12Fence> m_Fence;
	queue<shared_ptr<CommandList>> m_AvailableCommandLists;
	queue<CommandListEntry> m_InFlightCommandLists;
	atomic_u64 m_FenceValue = 0;
};