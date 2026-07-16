#pragma once
#include "pch.h"
#include "Mesh.h"
#include "VertexBufferManager.h"

class UploadManager : public Singleton<UploadManager>
{
	friend class Singleton<UploadManager>;
public:
	void QueueMeshForUpload(Mesh* mesh)
	{
		MeshesToUpload.push_back(mesh);
	}
	void UploadQueuedMeshes()
	{
		if (MeshesToUpload.size() == 0)
			return;

		u32 bytesRequired = 0;
		u32 uploadBufferOffset = 0;
		for (Mesh* mesh : MeshesToUpload)
		{
			if (!mesh->IsGPUDataAllocated)
			{
				FATAL_ERROR("Mesh data must be allocated on the GPU before uploading.");
			}
		}

		ID3D12Resource* vertexBufferResource = VertexBufferManager::GetAllocator()->GetResource();

		UINT8* mappedData = nullptr;
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(VertexUploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));

		auto queue = Graphics::GetDirectCommandQueue();
		auto commandList = queue->GetCommandList();
		commandList->Reset();
		auto commandListd3d12 = commandList->GetD3D12CommandList();

		for (Mesh* mesh : MeshesToUpload)
		{
			if (mesh->IsGPUDataUploaded)
				continue;

			int gpuOffsetToPositions = mesh->PositionsBufferAllocation.Offset;
			int gpuOffsetToColors = mesh->ColorsBufferAllocation.Offset;

			bytesRequired += ByteSize(mesh->Positions) + ByteSize(mesh->Colors);

			if (bytesRequired > UploadBufferSize)
			{
				FATAL_ERROR("Upload buffer is too small to upload all queued meshes.");
			}

			//Positions
			void* offset = mappedData + uploadBufferOffset;
			int byteSize = ByteSize(mesh->Positions);
			memcpy(offset,
				   mesh->Positions.data(),
				   byteSize);
			commandListd3d12->CopyBufferRegion(vertexBufferResource, mesh->PositionsBufferAllocation.Offset,
											   VertexUploadBuffer.Get(), uploadBufferOffset,
											   byteSize);

			uploadBufferOffset += byteSize;

			//Colors
			offset = mappedData + uploadBufferOffset;
			byteSize = ByteSize(mesh->Colors);
			memcpy(offset,
				   mesh->Colors.data(),
				   byteSize);
			commandListd3d12->CopyBufferRegion(vertexBufferResource, mesh->ColorsBufferAllocation.Offset,
											   VertexUploadBuffer.Get(), uploadBufferOffset,
											   byteSize);
			uploadBufferOffset += byteSize;


			mesh->IsGPUDataUploaded = true;
		}

		VertexUploadBuffer->Unmap(0, nullptr);

		//D3D12_BUFFER_BARRIER bufferBarrier{};

		//bufferBarrier.SyncBefore = D3D12_BARRIER_SYNC_COPY; //Before this barrier, there was COPY work touching this memory and we need that to finish.
		//bufferBarrier.SyncAfter = D3D12_BARRIER_SYNC_DRAW; //The next GPU work that will touch this memory is draw work and it will be dependent on this to be completed.

		//bufferBarrier.AccessBefore = D3D12_BARRIER_ACCESS_COPY_DEST; //The previous operation accessed this memory as a copy destination.
		//bufferBarrier.AccessAfter = D3D12_BARRIER_ACCESS_VERTEX_BUFFER; //The draw will access this memory through the vertex fetch hardware.

		//bufferBarrier.pResource = vertexBufferResource;
		//bufferBarrier.Offset = 0;
		//bufferBarrier.Size = UINT64_MAX;

		//D3D12_BARRIER_GROUP group{};

		//group.Type = D3D12_BARRIER_TYPE_BUFFER;
		//group.NumBarriers = 1;
		//group.pBufferBarriers = &bufferBarrier;

		//commandListd3d12->Barrier(1, &group);

		commandList->Close();

		queue->ExecuteCommandList(commandList);
		u64 fenceValue = queue->Signal();
		for(auto mesh : MeshesToUpload)
		{
			mesh->UploadFenceValue = fenceValue;
		}

		queue->WaitForFenceValue(fenceValue); //TODO: Remove this wait and instead check the fence value in the renderer before using the mesh.

		MeshesToUpload.clear();

	}
private:
	UploadManager(u32 uploadBufferSize = 16 * 1024 * 1024)
		:UploadBufferSize(uploadBufferSize)
	{
		//Create Upload Buffer
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(UploadBufferSize);

		//Create Vertex Upload Buffer
		CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);

		auto pDevice = Graphics::GetDevice();

		ThrowIfFailed(pDevice->CreateCommittedResource(
			&uploadHeap,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&VertexUploadBuffer)));		
	}
	vector<Mesh*> MeshesToUpload;
	u32 UploadBufferSize = 16 * 1024 * 1024; //16MB
	ComPtr<ID3D12Resource> VertexUploadBuffer;

};