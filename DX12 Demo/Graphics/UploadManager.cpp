#include "pch.h"
#include "UploadManager.h"
#include "Graphics.h"

UploadManager::UploadManager(u32 uploadBufferSize)
	: UploadBufferSize(uploadBufferSize)
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
		IID_PPV_ARGS(&UploadBuffer)));

	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(UploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&MappedUploadBuffer)));

}

void UploadManager::QueueMeshForUpload(Mesh* mesh)
{
	MeshesToUpload.push_back(mesh);
}

void UploadManager::QueueTextureForUpload(Texture* texture)
{
	TexturesToUpload.push_back(texture);
}

void UploadManager::ResetUploadBufferOffset()
{
	UploadBufferOffset = 0;
}

void UploadManager::UploadQueuedMeshes()
{
	PROFILE_FUNCTION();

	if (MeshesToUpload.size() == 0)
		return;

	u32 bytesRequired = 0;
	for (Mesh* mesh : MeshesToUpload)
	{
		if (!mesh->IsGPUDataAllocated)
		{
			FATAL_ERROR("Mesh data must be allocated on the GPU before uploading.");
		}
	}

	ID3D12Resource* vertexBufferResource = VertexBufferManager::GetAllocator()->GetResource();

	auto queue = Graphics::GetTransferCommandQueue();
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
		void* offset = MappedUploadBuffer + UploadBufferOffset;
		int byteSize = ByteSize(mesh->Positions);
		memcpy(offset,
				mesh->Positions.data(),
				byteSize);
		commandListd3d12->CopyBufferRegion(vertexBufferResource, mesh->PositionsBufferAllocation.Offset,
											UploadBuffer.Get(), UploadBufferOffset,
											byteSize);

		UploadBufferOffset += byteSize;

		//Colors
		offset = MappedUploadBuffer + UploadBufferOffset;
		byteSize = ByteSize(mesh->Colors);
		memcpy(offset,
				mesh->Colors.data(),
				byteSize);
		commandListd3d12->CopyBufferRegion(vertexBufferResource, mesh->ColorsBufferAllocation.Offset,
											UploadBuffer.Get(), UploadBufferOffset,
											byteSize);
		UploadBufferOffset += byteSize;

		//TexCoords
		offset = MappedUploadBuffer + UploadBufferOffset;
		byteSize = ByteSize(mesh->TexCoords);
		memcpy(offset,
			   mesh->TexCoords.data(),
			   byteSize);
		commandListd3d12->CopyBufferRegion(vertexBufferResource, mesh->TexCoordsBufferAllocation.Offset,
										   UploadBuffer.Get(), UploadBufferOffset,
										   byteSize);
		UploadBufferOffset += byteSize;

		mesh->IsGPUDataUploaded = true;
	}

	commandList->Close();

	queue->ExecuteCommandList(commandList);
	u64 fenceValue = queue->Signal();
	for (auto mesh : MeshesToUpload)
	{
		mesh->UploadFenceValue = fenceValue;
	}

	MeshesToUpload.clear();
}

void UploadManager::UploadQueuedTextures()
{
	PROFILE_FUNCTION();

	if (TexturesToUpload.size() == 0)
		return;

	auto device = Graphics::GetDevice();

	CD3DX12_RANGE readRange(0, 0);

	auto queue = Graphics::GetTransferCommandQueue();
	auto commandList = queue->GetCommandList();
	commandList->Reset();
	auto commandListd3d12 = commandList->GetD3D12CommandList();

	for (const auto& texture : TexturesToUpload)
	{
		if (texture->IsGPUDataUploaded)
			continue;

		if (UploadBufferOffset + texture->Data.size() > UploadBufferSize)
		{
			FATAL_ERROR("Upload buffer is too small to upload texture. Need to wait for GPU fence.");
		}

		// Copy texture data into upload buffer
		void* offset = MappedUploadBuffer + UploadBufferOffset;
		memcpy(offset, texture->Data.data(), texture->Data.size());

		// Get footprint for texture copy
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width = texture->Width;
		desc.Height = texture->Height;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = texture->Format;
		desc.SampleDesc.Count = 1;

		UINT64 requiredSize;
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
		UINT numRows;
		UINT64 rowSizeInBytes;

		device->GetCopyableFootprints(&desc, 0, 1, UploadBufferOffset, &footprint, &numRows, &rowSizeInBytes, &requiredSize);

		// Copy from upload buffer to texture
		D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
		srcLocation.pResource = UploadBuffer.Get();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		srcLocation.PlacedFootprint = footprint;

		D3D12_TEXTURE_COPY_LOCATION destLocation = {};
		destLocation.pResource = texture->Resource.Get();
		destLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		destLocation.SubresourceIndex = 0;

		commandListd3d12->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, nullptr);

		UploadBufferOffset += texture->Data.size();
		texture->IsGPUDataUploaded = true;
	}

	commandList->Close();

	queue->ExecuteCommandList(commandList);
	u64 fenceValue = queue->Signal();

	TexturesToUpload.clear();
}