#include "pch.h"
#include "Mesh.h"
#include "VertexBufferManager.h"

void Mesh::BuildBufferAllocations() //This should onl be called once per mesh, and only after the Positions and Colors vectors have been filled with data. It allocates GPU memory for the mesh's vertex data and sets up the corresponding buffer views.
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
    if (TexCoords.size() > 0)
    {
        TexCoordsBufferAllocation = allocator->Allocate(ByteSize(TexCoords), 4);
        TexCoordBufferView.BufferLocation = base + TexCoordsBufferAllocation.Offset;
        TexCoordBufferView.StrideInBytes = sizeof(TexCoords[0]);
        TexCoordBufferView.SizeInBytes = ByteSize(TexCoords);
    }

    IsGPUDataAllocated = true;
}
 