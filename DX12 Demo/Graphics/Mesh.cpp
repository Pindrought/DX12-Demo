#include "pch.h"
#include "Mesh.h"
#include "VertexBufferManager.h"

void Mesh::BuildBufferAllocations()
{
    if (IsGPUDataAllocated)
    {
		DBG_LOG("[Mesh::BuildBufferAllocations()] Mesh data has already been allocated on the GPU.");
        return; //Already allocated?
    }
    auto allocator = VertexBufferManager::GetAllocator();
    auto base = allocator->GetResource()->GetGPUVirtualAddress();
    if (Positions.size() > 0)
    {
        PositionsBufferAllocation = allocator->Allocate(ByteSize(Positions), 4);
        PositionBufferView.BufferLocation = base + PositionsBufferAllocation.Offset;
        PositionBufferView.StrideInBytes = sizeof(Positions[0]);
        PositionBufferView.SizeInBytes = ByteSize(Positions);
    }
    if (Colors.size() > 0)
    {
        ColorsBufferAllocation = allocator->Allocate(ByteSize(Colors), 4);
        ColorBufferView.BufferLocation = base + ColorsBufferAllocation.Offset;
        ColorBufferView.StrideInBytes = sizeof(Colors[0]);
        ColorBufferView.SizeInBytes = ByteSize(Colors);
    }
    IsGPUDataAllocated = true;
}
 