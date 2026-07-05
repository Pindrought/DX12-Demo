#include "pch.h"
#include "VertexBufferManager.h"

BufferAllocator VertexBufferManager::s_Allocator;
bool VertexBufferManager::s_Initialized = false;

void VertexBufferManager::Initialize()
{
	if (s_Initialized)
	{
		DBG_LOG("VertexBufferManager::Initialize called when already initialized.");
		throw std::exception("VertexBufferManager::Initialize called when already initialized.");
	}
	s_Allocator.Initialize(1024 * 1024 * 64,
						   D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE,
						   D3D12_RESOURCE_STATE_COMMON);
	s_Initialized = true;
}

BufferAllocator* VertexBufferManager::GetAllocator()
{
	if (!s_Initialized)
	{
		DBG_LOG("[VertexBufferManager::GetAllocator()] called before initialization.");
		throw std::exception("[VertexBufferManager::GetAllocator()] called before initialization.");
	}
	return &s_Allocator;
}
