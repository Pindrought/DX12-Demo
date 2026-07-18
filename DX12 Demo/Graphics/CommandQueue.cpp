#include "pch.h"
#include "CommandQueue.h"
#include "Graphics.h"
#include "Profiler.h"

CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type)
	: m_CommandListType(type)
{
	auto pDevice = Graphics::GetDevice();

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	ThrowIfFailed(pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue)));
	ThrowIfFailed(pDevice->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
}

ID3D12CommandQueue* CommandQueue::GetD3D12CommandQueue() const
{
	return m_CommandQueue.Get();
}

shared_ptr<CommandList> CommandQueue::GetCommandList()
{
	std::shared_ptr<CommandList> commandList;

	// If there is a command list on the queue.
	if (!m_AvailableCommandLists.empty())
	{
		commandList = m_AvailableCommandLists.front();
		m_AvailableCommandLists.pop();
	}
	else
	{
		// Otherwise create a new command list.
		commandList = std::make_shared<CommandList>(m_CommandListType);
	}

	return commandList;
}

u64 CommandQueue::ExecuteCommandList(shared_ptr<CommandList> commandList)
{
	return ExecuteCommandLists(vector<shared_ptr<CommandList>>({ commandList }));
}

u64 CommandQueue::ExecuteCommandLists(const vector<shared_ptr<CommandList>>& commandLists)
{
	PROFILE_FUNCTION();
	// Command lists that need to be executed.
	std::vector<ID3D12CommandList*> d3d12CommandLists;
	d3d12CommandLists.reserve(commandLists.size());

	for (auto commandList : commandLists)
	{
		d3d12CommandLists.push_back(commandList->GetD3D12CommandList());
	}

    UINT numCommandLists = static_cast<UINT>(d3d12CommandLists.size());
    m_CommandQueue->ExecuteCommandLists(numCommandLists, d3d12CommandLists.data());

    u64 fenceValue = Signal();

    // Queue command lists for reuse.
    for (auto commandList : commandLists)
    {
        m_InFlightCommandLists.push({ fenceValue, commandList });
    }

    return fenceValue;
}

bool CommandQueue::IsFenceComplete(u64 fenceValue)
{
	return m_Fence->GetCompletedValue() >= fenceValue;
}

u64 CommandQueue::Signal()
{
	u64 fenceValue = ++m_FenceValue;
	m_CommandQueue->Signal(m_Fence.Get(), fenceValue);
	return fenceValue;
}

void CommandQueue::WaitForFenceValue(u64 fenceValue)
{
	if (!IsFenceComplete(fenceValue))
	{
		HANDLE eventHandle = ::CreateEvent(nullptr, 
										   false, //Auto reset
										   false, //Non signaled
										   nullptr);
		if (!eventHandle)
		{
			throw std::runtime_error("[CommandQueue::WaitForFenceValue(u64 fenceValue)] Failed to create fence event.");
		}
		ThrowIfFailed(m_Fence->SetEventOnCompletion(fenceValue, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void CommandQueue::RecycleInFlightCommandLists()
{
	PROFILE_FUNCTION();

	while (m_InFlightCommandLists.empty() == false)
	{
		auto entry = m_InFlightCommandLists.front();
		if (IsFenceComplete(entry.FenceValue))
		{
			m_InFlightCommandLists.pop();
			m_AvailableCommandLists.push(entry.CommandList);
		}
		else //Fence not complete? try to recycle it next frame then
		{
			break;
		}
	}
}
