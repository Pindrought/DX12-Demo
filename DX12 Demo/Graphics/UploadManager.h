#pragma once
#include "pch.h"
#include "Mesh.h"
#include "Texture.h"
#include "VertexBufferManager.h"
#include "Profiler.h"

class UploadManager : public Singleton<UploadManager>
{
	friend class Singleton<UploadManager>;
	UploadManager(u32 uploadBufferSize = 256 * 1024 * 1024);


public:
	void QueueMeshForUpload(Mesh* mesh);
	void QueueTextureForUpload(Texture* texture);

	void ResetUploadBufferOffset();
	void UploadQueuedMeshes();
	void UploadQueuedTextures();
private:
	
	vector<Mesh*> MeshesToUpload;
	vector<Texture*> TexturesToUpload;

	u32 UploadBufferSize = 256 * 1024 * 1024; //256
	ComPtr<ID3D12Resource> UploadBuffer;
	u8* MappedUploadBuffer = nullptr;
	u32 UploadBufferOffset = 0;
};