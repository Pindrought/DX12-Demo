#include "pch.h"
#include "Renderer.h"
#include "Graphics.h"
#include "../Window/Window.h"

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
	static int frameCount = 0;
	static Timer* timer = nullptr;
	if (timer == nullptr)
	{
		timer = new Timer();
		timer->Start();
	}
	auto commandList = PopulateCommandList(pWindow);
	auto commandQueue = m_Graphics.GetDirectCommandQueue();
	commandQueue->RecycleInFlightCommandLists();

	u64 fenceValue = commandQueue->ExecuteCommandList(commandList);
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
	
	pWindow->m_FrameIndex = pWindow->m_SwapChain->GetCurrentBackBufferIndex();

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

void Renderer::InitializeAssets()
{
	auto pDevice = Graphics::GetDevice();
	// Create an empty root signature.
	{
		/*
		/*CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		ThrowIfFailed(pDevice->CreateRootSignature(0, 
												   signature->GetBufferPointer(), 
												   signature->GetBufferSize(), 
												   IID_PPV_ARGS(&m_RootSignature)));*/
												   


		// Create a root signature.
	// Allow input layout and deny unnecessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
														D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
														D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
														D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;


		CD3DX12_ROOT_PARAMETER1 rootParameters[1];
		rootParameters[0].InitAsConstantBufferView(0, //Register b0
												   0, //Space    space0
												   D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
												   D3D12_SHADER_VISIBILITY_VERTEX);

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
		UINT8* pVertexShaderData = nullptr;
		UINT8* pPixelShaderData = nullptr;
		UINT vertexShaderDataLength = 0;
		UINT pixelShaderDataLength = 0;

		ThrowIfFailed(ReadDataFromFile(L"C:\\Users\\Jacob\\source\\repos\\DX12 Demo\\x64\\Debug\\vs.cso", &pVertexShaderData, &vertexShaderDataLength));
		ThrowIfFailed(ReadDataFromFile(L"C:\\Users\\Jacob\\source\\repos\\DX12 Demo\\x64\\Debug\\ps.cso", &pPixelShaderData, &pixelShaderDataLength));

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_RootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(pVertexShaderData, vertexShaderDataLength);
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pPixelShaderData, pixelShaderDataLength);
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

	// Create the vertex buffer.
	{
		// Define the geometry for a triangle.
		float aspectRatio = m_ViewPort.Width / m_ViewPort.Height;
		Vertex triangleVertices[] =
		{
			{ { 0.0f, 0.25f * aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f * aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f * aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		const UINT vertexBufferSize = sizeof(triangleVertices);

		// Note: using upload heaps to transfer static data like vert buffers is not 
		// recommended. Every time the GPU needs it, the upload heap will be marshalled 
		// over. Please read up on Default Heap usage. An upload heap is used here for 
		// code simplicity and because there are very few verts to actually transfer.
		auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
		ThrowIfFailed(pDevice->CreateCommittedResource(&heapProps,
													   D3D12_HEAP_FLAG_NONE,
													   &resourceDesc,
													   D3D12_RESOURCE_STATE_GENERIC_READ,
													   nullptr,
													   IID_PPV_ARGS(&m_VertexBuffer)));

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_VertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		m_VertexBuffer->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
		m_VertexBufferView.StrideInBytes = sizeof(Vertex);
		m_VertexBufferView.SizeInBytes = vertexBufferSize;
	}
}

shared_ptr<CommandList> Renderer::PopulateCommandList(Window* pWindow)
{
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
	auto frameIndex = pWindow->m_FrameIndex;
	auto renderTarget = pWindow->m_RenderTargets[frameIndex].Get();

	auto barrier_present_to_rt = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget,
														D3D12_RESOURCE_STATE_PRESENT,
														D3D12_RESOURCE_STATE_RENDER_TARGET);

	cmdListd3d->ResourceBarrier(1, &barrier_present_to_rt);

	auto rtvHeap = pWindow->m_RTVHeap.Get();
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), 
											frameIndex, 
											pWindow->m_RTVDescriptorSize);
	cmdListd3d->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	static bool timerElapsed = false;
	static Timer timer;
	timer.Start();
	float elapsed = timer.GetMilisecondsElapsed();
	if (elapsed > 1000.0f)
	{
		timer.Restart();
		timerElapsed = !timerElapsed;
	}

	float r = 1;
	if (timerElapsed)
	{
		r = 0;
	}
	const float clearColor[] = { r, 0.2f, 0.4f, 1.0f };
	cmdListd3d->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	cmdListd3d->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdListd3d->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	cmdListd3d->DrawInstanced(3, 1, 0, 0);

	// Indicate that the back buffer will now be used to present.
	auto barrier_rt_to_present = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget,
																	  D3D12_RESOURCE_STATE_RENDER_TARGET,
																	  D3D12_RESOURCE_STATE_PRESENT);
	cmdListd3d->ResourceBarrier(1, &barrier_rt_to_present);
	commandList->Close();

	return commandList;
}

void Renderer::GoToNextFrame()
{
	m_FrameIndex++;
	m_FrameIndex %= NUMBER_FRAMES_IN_FLIGHT;
	auto commandQueue = Graphics::GetDirectCommandQueue();
	if (commandQueue->IsFenceComplete(m_FenceValues[m_FrameIndex]))
	{
		//DBG_LOG("Fence ready: " + std::to_string(m_FenceValues[m_FrameIndex]));
	}
	else
	{
		//DBG_LOG("Waiting for fence value: " + std::to_string(m_FenceValues[m_FrameIndex]));
		commandQueue->WaitForFenceValue(m_FenceValues[m_FrameIndex]);
	}
}