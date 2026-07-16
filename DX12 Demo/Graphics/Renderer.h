#pragma once
#include "pch.h"
#include "Graphics.h"
#include "Mesh.h"
#include "UploadManager.h"

class Renderer
{
public:
	bool Initialize();
	void Render(Window* pWindow);
private:
	void InitializeAssets();
	shared_ptr<CommandList> PopulateCommandList(Window* pWindow);
	void GoToNextFrame();
	Graphics m_Graphics;
	ComPtr<ID3D12PipelineState> m_PipelineState;
	ComPtr<ID3D12RootSignature> m_RootSignature;
	CD3DX12_VIEWPORT m_ViewPort;
	CD3DX12_RECT m_ScissorRect;


	u8 m_FrameIndex = 0;
	u64 m_FenceValues[NUMBER_FRAMES_IN_FLIGHT] = {};
	u64 m_FramesRendered = 0;

	//Mesh TriangleMesh;

	std::vector<Mesh*> Meshes;
};

