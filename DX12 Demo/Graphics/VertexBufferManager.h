#pragma once
#include "pch.h"
#include "BufferAllocator.h"

class VertexBufferManager
{
public:
	static void Initialize();
	static BufferAllocator* GetAllocator();
private:
	static BufferAllocator s_Allocator;
	static bool s_Initialized;
};