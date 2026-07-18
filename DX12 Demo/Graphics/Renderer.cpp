#include "pch.h"
#include "Renderer.h"
#include "Graphics.h"
#include "../Window/Window.h"
#include "ShaderManager.h"
#include "VertexBufferManager.h"
#include "../IO/DirectoryHelper.h"
#include "Profiler.h"

Renderer* s_Instance = nullptr;

bool Renderer::Initialize()
{
	DBG_LOG("[Renderer::Initialize()]");

	if (s_Instance != nullptr)
	{
		DBG_LOG("[Renderer::Initialize()]  Renderer already initialized.");
		throw std::runtime_error("[Renderer::Initialize()]  Renderer already initialized.");
	}

	m_Graphics.Initialize();
	UploadManager::Initialize();
	m_UploadRingBuffer.Initialize(256 * 10000);
	auto pDevice = Graphics::GetDevice();

	m_ViewPort = CD3DX12_VIEWPORT(0.0f,
								  0.0f,
								  800.0f,
								  600.0f);
	m_ScissorRect = CD3DX12_RECT(0, 0, 800, 600);

	InitializeAssets();

	s_Instance = this;
	return true;
}

void Renderer::Render(Window* pWindow)
{
	PROFILE_FUNCTION();
	u64 fenceValue = 0;
	static int frameCount = 0;

	static Timer* timer = nullptr;

	WaitForSingleObject(pWindow->GetSwapChainWaitableObject(), INFINITE);

	{
		PROFILE_SCOPE("Renderer::Render -> Build & execute command lists");

		if (timer == nullptr)
		{
			timer = new Timer();
			timer->Start();
		}

		auto& uploadManager = UploadManager::Get();
		uploadManager.UploadQueuedMeshes();

		u64 completedFenceValue = Graphics::GetDirectCommandQueue()->m_Fence->GetCompletedValue();
		m_UploadRingBuffer.ReclaimCompletedSubmissions(completedFenceValue);
		m_UploadRingBuffer.BeginSubmission();

		auto commandQueue = m_Graphics.GetDirectCommandQueue();

		auto commandList = PopulateCommandList(pWindow);
		commandQueue->RecycleInFlightCommandLists();

		if (commandList->m_RequiredTransferFenceValue > 0)
		{
			auto transferQueue = Graphics::GetTransferCommandQueue();
			commandQueue->GetD3D12CommandQueue()->Wait(transferQueue->m_Fence.Get(), commandList->m_RequiredTransferFenceValue);
		}

		fenceValue = commandQueue->ExecuteCommandList(commandList);
		m_UploadRingBuffer.EndSubmission(fenceValue);
	}
	
	Present(pWindow, fenceValue);

	{
		PROFILE_SCOPE("Renderer::Render -> GoToNextFrame");
		m_FenceValues[m_FrameIndex] = fenceValue;

		GoToNextFrame();

		frameCount++;
		if (timer->GetMilisecondsElapsed() > 1000.0f)
		{
			float fps = frameCount / (timer->GetMilisecondsElapsed() / 1000.0f);
			DBG_LOG(sfmt("[Renderer::Render()] FPS: [%f]", fps));
			timer->Restart();
			frameCount = 0;
		}
	}
}

void Renderer::Present(Window* pWindow, u64 fenceValue)
{
	PROFILE_FUNCTION();
	auto start = std::chrono::high_resolution_clock::now();

	HRESULT hr;

	if (Graphics::IsVSyncOn())
	{
		ThrowIfFailed(pWindow->m_SwapChain->Present(1, 0));
	}
	else
	{
		if (Graphics::IsTearingSupported())
		{
			ThrowIfFailed(pWindow->m_SwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING));
		}
		else
		{
			ThrowIfFailed(pWindow->m_SwapChain->Present(0, 0));
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count();

	if (ms > 100.0)
	{
		auto fence = Graphics::GetDirectCommandQueue()->m_Fence;
		DBG_LOG(sfmt("PRESENT SPIKE: %.2f ms | Submitted: %llu | Completed: %llu | BackBuffer: %u",
					 ms,
					 fenceValue,
					 fence->GetCompletedValue(),
					 pWindow->m_SwapChain->GetCurrentBackBufferIndex()));
		DebugBreak();
	}
}

void Renderer::InitializeAssets()
{
	auto pDevice = Graphics::GetDevice();
	// Create an empty root signature.
	{
		// Allow input layout and deny unnecessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
														D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
														D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
														D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;


		CD3DX12_ROOT_PARAMETER1 rootParameters[1];
		rootParameters[0].InitAsConstantBufferView(0, //Register b0
												   0, //Space    space0
												   D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
												   D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_STATIC_SAMPLER_DESC linearRepeatSampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
		CD3DX12_STATIC_SAMPLER_DESC anisotropicSampler(0, D3D12_FILTER_ANISOTROPIC);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
		rootSignatureDescription.Init_1_1(_countof(rootParameters), 
										  rootParameters, 
										  1, //# of samplers
										  &linearRepeatSampler, //ptr to samplers
										  rootSignatureFlags);

		D3D_ROOT_SIGNATURE_VERSION highestVersion = Graphics::GetHighestRootSignatureVersion();

		// Serialize the root signature.
		Microsoft::WRL::ComPtr<ID3DBlob> rootSignatureBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription, 
															highestVersion,
															&rootSignatureBlob, 
															&errorBlob));

		// Create the root signature.
		ThrowIfFailed(pDevice->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
													   rootSignatureBlob->GetBufferSize(),
													   IID_PPV_ARGS(&m_RootSignature)));

	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		ShaderManager::LoadCSO("vs", DirectoryHelper::GetExecutableDirectory().string() + "\\vs.cso");
		ShaderManager::LoadCSO("ps", DirectoryHelper::GetExecutableDirectory().string() + "\\ps.cso");

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_RootSignature.Get();

		auto vs = ShaderManager::GetShader("vs");
		auto ps = ShaderManager::GetShader("ps");

		psoDesc.VS = vs->GetBytecode();
		psoDesc.PS = ps->GetBytecode();
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState)));
	}

	//Create Mesh
	{
		Mesh* TriangleMesh = new Mesh();
		TriangleMesh->Positions = {
			{ 0.0f, 1, 0.0f }, //topmid
			{ 1, -1, 0.0f }, //bottomright
			{ -1, -1, 0.0f } //bottomleft
		};

		TriangleMesh->Colors = {
			 { 1.0f, 0.0f, 0.0f, 1.0f },
			 { 0.0f, 1.0f, 0.0f, 1.0f },
			 { 0.0f, 0.0f, 1.0f, 1.0f }
		};

		TriangleMesh->BuildBufferAllocations();
		Meshes.push_back(TriangleMesh);

		auto& uploadManager = UploadManager::Get();
		uploadManager.QueueMeshForUpload(Meshes.back());
	}
}

shared_ptr<CommandList> Renderer::PopulateCommandList(Window* pWindow)
{
	PROFILE_FUNCTION();

	auto pDevice = Graphics::GetDevice();
	auto pDirectCommandQueue = Graphics::GetDirectCommandQueue();
	auto commandList = pDirectCommandQueue->GetCommandList();
	commandList->Reset();

	auto& cmdListd3d = commandList->m_CommandList;

	cmdListd3d->SetPipelineState(m_PipelineState.Get());

	// Set necessary state.
	cmdListd3d->SetGraphicsRootSignature(m_RootSignature.Get());

	cmdListd3d->RSSetViewports(1, &m_ViewPort);
	cmdListd3d->RSSetScissorRects(1, &m_ScissorRect);

	// Indicate that the back buffer will be used as a render target.
	auto renderTarget = pWindow->m_RenderTargets[pWindow->GetBackBufferIndex()].Get();

	/*auto barrier_present_to_rt = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget,
														D3D12_RESOURCE_STATE_PRESENT,
														D3D12_RESOURCE_STATE_RENDER_TARGET);

	cmdListd3d->ResourceBarrier(1, &barrier_present_to_rt);*/

	{
		D3D12_TEXTURE_BARRIER renderTargetBarrier = {};
		renderTargetBarrier.pResource = renderTarget;
		renderTargetBarrier.SyncBefore = D3D12_BARRIER_SYNC_NONE;
		renderTargetBarrier.SyncAfter = D3D12_BARRIER_SYNC_RENDER_TARGET;
		renderTargetBarrier.AccessBefore = D3D12_BARRIER_ACCESS_NO_ACCESS;
		renderTargetBarrier.AccessAfter = D3D12_BARRIER_ACCESS_RENDER_TARGET;
		renderTargetBarrier.LayoutBefore = D3D12_BARRIER_LAYOUT_PRESENT;
		renderTargetBarrier.LayoutAfter = D3D12_BARRIER_LAYOUT_RENDER_TARGET;
		renderTargetBarrier.Subresources.IndexOrFirstMipLevel = 0xFFFFFFFF;

		D3D12_BARRIER_GROUP barrierGroup = {};
		barrierGroup.Type = D3D12_BARRIER_TYPE_TEXTURE;
		barrierGroup.NumBarriers = 1;
		barrierGroup.pTextureBarriers = &renderTargetBarrier;

		cmdListd3d->Barrier(1, &barrierGroup);
	}


	auto rtvHeap = pWindow->m_RTVHeap.Get();
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), 
											pWindow->GetBackBufferIndex(),
											pWindow->m_RTVDescriptorSize);
	cmdListd3d->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	static bool timerElapsed = false;
	static Timer timer;
	timer.Start();
	float elapsed = timer.GetMilisecondsElapsed();

	//Code to flip cbuffer from colored to non-colored every 2 seconds.
	float cBufferDataProgress = fmod(elapsed, 4000.0f);
	if (cBufferDataProgress > 2000)
	{
		m_PerObjectConstantBufferData.HasColoredVertices = TRUE;
	}
	else
	{
		m_PerObjectConstantBufferData.HasColoredVertices = FALSE;
	}

	ConstantBufferAllocation cBufferAlloc = m_UploadRingBuffer.Allocate(sizeof(m_PerObjectConstantBufferData));
	cmdListd3d->SetGraphicsRootConstantBufferView(0,
												  cBufferAlloc.GPUAddress);
	memcpy(cBufferAlloc.CPUAddress, &m_PerObjectConstantBufferData, sizeof(m_PerObjectConstantBufferData));

	//Code to make background color pulse every second
	float r = 1;
	float elapsedMod = fmod(elapsed, 2000.0f);
	float elapsedVal = elapsedMod;
	if (elapsedMod > 1000)
	{
		elapsedVal = 2000 - elapsedMod;
	}
	float progress = elapsedVal / 1000.0f;
	r = progress * 1;
	const float clearColor[] = { r, 0.2f, 0.4f, 1.0f };
	cmdListd3d->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	//Draw meshes
	cmdListd3d->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for(auto mesh : Meshes)
	{
		if (mesh->UploadFenceValue > commandList->m_RequiredTransferFenceValue)
		{
			commandList->m_RequiredTransferFenceValue = mesh->UploadFenceValue;
		}
		const D3D12_VERTEX_BUFFER_VIEW bufferViews[] = {
			mesh->PositionBufferView,
			mesh->ColorBufferView
		};
		cmdListd3d->IASetVertexBuffers(0, 2, bufferViews);
		cmdListd3d->DrawInstanced(3, 1, 0, 0);
	}

	// Indicate that the back buffer will now be used to present.
	//auto barrier_rt_to_present = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget,
	//																  D3D12_RESOURCE_STATE_RENDER_TARGET,
	//																  D3D12_RESOURCE_STATE_PRESENT);

	//cmdListd3d->ResourceBarrier(1, &barrier_rt_to_present);

	{
		D3D12_TEXTURE_BARRIER renderTargetBarrier = {};
		renderTargetBarrier.pResource = renderTarget;
		renderTargetBarrier.SyncBefore = D3D12_BARRIER_SYNC_RENDER_TARGET;
		renderTargetBarrier.SyncAfter = D3D12_BARRIER_SYNC_NONE;
		renderTargetBarrier.AccessBefore = D3D12_BARRIER_ACCESS_RENDER_TARGET;
		renderTargetBarrier.AccessAfter = D3D12_BARRIER_ACCESS_NO_ACCESS;
		renderTargetBarrier.LayoutBefore = D3D12_BARRIER_LAYOUT_RENDER_TARGET; 
		renderTargetBarrier.LayoutAfter = D3D12_BARRIER_LAYOUT_PRESENT;
		renderTargetBarrier.Subresources.IndexOrFirstMipLevel = 0xFFFFFFFF;

		D3D12_BARRIER_GROUP barrierGroup = {};
		barrierGroup.Type = D3D12_BARRIER_TYPE_TEXTURE;
		barrierGroup.NumBarriers = 1;
		barrierGroup.pTextureBarriers = &renderTargetBarrier;

		cmdListd3d->Barrier(1, &barrierGroup);
	}

	commandList->Close();

	return commandList;
}

void Renderer::GoToNextFrame()
{
	PROFILE_FUNCTION();

	m_FrameIndex++;
	m_FrameIndex %= NUMBER_FRAMES_IN_FLIGHT;
	m_FramesRendered += 1;
	auto commandQueue = Graphics::GetDirectCommandQueue();

	if (commandQueue->IsFenceComplete(m_FenceValues[m_FrameIndex]))
	{
		//DBG_LOG("Fence ready: " + std::to_string(m_FenceValues[m_FrameIndex]));
	}
	else
	{
		Timer t;
		t.Start();
		commandQueue->WaitForFenceValue(m_FenceValues[m_FrameIndex]);
		float elapsed = t.GetMilisecondsElapsed();
		if (t.GetMilisecondsElapsed() > 1) //If we wait over 1 sec on a fence, log it as this should not happen.
		{
			DBG_LOG(sfmt("Completed fence value: %d | elapsed: %f", m_FenceValues[m_FrameIndex], elapsed));
		}
	}

	if (m_FramesRendered > 500) //Adding new mesh after 500 frames
	{
		static bool triangle2 = false;
		if (triangle2 == false)
		{
			triangle2 = true;

			Mesh* TriangleMesh = new Mesh();
			TriangleMesh->Positions = {
				{ -1.0f, 1, 0.0f }, //topmid
				{ 1, -1, 0.0f }, //bottomright
				{ -1, -1, 0.0f } //bottomleft
			};

			TriangleMesh->Colors = {
				 { 1.0f, 1.0f, 1.0f, 1.0f },
				 { 0.0f, 1.0f, 0.0f, 1.0f },
				 { 1.0f, 0.0f, 1.0f, 1.0f }
			};

			TriangleMesh->BuildBufferAllocations();
			Meshes.push_back(TriangleMesh);

			auto& uploadManager = UploadManager::Get();
			uploadManager.QueueMeshForUpload(Meshes.back());
		}
	}
}

