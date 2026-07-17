#include "pch.h"
#include "Graphics.h"
#include "VertexBufferManager.h"

Graphics* s_Instance = nullptr;

void Graphics::Initialize(bool useWarp)
{
	if (s_Instance != nullptr)
	{
		DBG_LOG("[Graphics::Initialize()]  Graphics already initialized.");
		throw std::runtime_error("[Graphics::Initialize()]  Graphics already initialized.");
	}
	DBG_LOG("[Graphics::Initialize()]");
	s_Instance = this;

	m_UseWarpDevice = useWarp;
	DBG_LOG(sfmt("[Graphics::Initialize()] -> UseWarp: [%d]", m_UseWarpDevice));

	EnableDebugLayer();
	InitializeDXGIFactory();
	InitializeDevice();

	m_DirectCommandQueue = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
	m_TransferCommandQueue = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COPY);

	VertexBufferManager::Initialize();
}

Graphics* Graphics::GetInstance()
{
	if (s_Instance == nullptr)
	{
		DBG_LOG("[Graphics::Get()] Error: Graphics instance not initialized.");
		throw std::runtime_error("[Graphics::Get()]  Graphics instance not initialized.");
		return nullptr;
	}
	return s_Instance;
}

CommandQueue* Graphics::GetDirectCommandQueue()
{
	return GetInstance()->m_DirectCommandQueue.get();
}

CommandQueue* Graphics::GetTransferCommandQueue()
{
	return GetInstance()->m_TransferCommandQueue.get();
}

ID3D12Device2* Graphics::GetDevice()
{
	return GetInstance()->m_Device.Get();
}

IDXGIFactory4* Graphics::GetDXGIFactory()
{
	return GetInstance()->m_DXGIFactory.Get();
}

D3D_ROOT_SIGNATURE_VERSION Graphics::GetHighestRootSignatureVersion()
{
	return GetInstance()->m_HighestRootSignatureVersion;
}

bool Graphics::IsTearingSupported()
{
	return GetInstance()->m_TearingSupported;
}

bool Graphics::IsVSyncOn()
{
	return GetInstance()->m_Vsync;
}

void Graphics::EnableDebugLayer()
{
#if defined(_DEBUG)
	if (m_DebugLayerEnabled)
	{
		DBG_LOG("[Graphics::EnableDebugLayer()] Enabling D3D12 Debug Layer");
		ComPtr<ID3D12Debug> debugInterface;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
		debugInterface->EnableDebugLayer();
	}
#endif
}

void Graphics::InitializeDXGIFactory()
{
	u32 dxgiFactoryFlags = 0;
#if defined(_DEBUG)
	if (m_DebugLayerEnabled)
	{
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG; // Enable additional debug layers.
	}
#endif
	//CreateDXGIFactory2 is requesting modern DXGI 1.3+ + support flags. It adds DXGI_CREATE_FACTORY_DEBUG support.
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_DXGIFactory)));

	BOOL allowTearing = false;
	if (SUCCEEDED(m_DXGIFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(BOOL))))
	{
		m_TearingSupported = (allowTearing == TRUE);
	}
}

void Graphics::InitializeDevice()
{
	DBG_LOG("[Graphics::InitializeDevice()]");
	ComPtr<IDXGIAdapter> adapter = nullptr;
	if (m_UseWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(m_DXGIFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(),
										D3D_FEATURE_LEVEL_11_0,
										IID_PPV_ARGS(&m_Device) ));
		adapter = warpAdapter;
	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter = GetHardwareAdapter();

		if (hardwareAdapter == nullptr)
		{
			DBG_LOG("[Graphics::InitializeDevice()] -> Failed to identify hardware adapter.");
			throw std::runtime_error("No valid hardware adapter found.");
		}

		ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(),
										D3D_FEATURE_LEVEL_11_0,
										IID_PPV_ARGS(&m_Device)
		));

		adapter = hardwareAdapter;
	}

	D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
	allocatorDesc.pDevice = m_Device.Get();
	allocatorDesc.pAdapter = adapter.Get();
	allocatorDesc.Flags = D3D12MA_RECOMMENDED_ALLOCATOR_FLAGS;

	ThrowIfFailed(D3D12MA::CreateAllocator(&allocatorDesc, &m_Allocator));


	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData;
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData,
													  sizeof(D3D12_FEATURE_DATA_ROOT_SIGNATURE))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}
		m_HighestRootSignatureVersion = featureData.HighestVersion;
	}

}

ComPtr<IDXGIAdapter1> Graphics::GetHardwareAdapter()
{
    ComPtr<IDXGIFactory6> factory6;
	ComPtr<IDXGIAdapter1> adapter;
    if (SUCCEEDED(m_DXGIFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        for (u32 adapterIndex = 0; ; adapterIndex++)
        {
            HRESULT hr = factory6->EnumAdapterByGpuPreference(adapterIndex,
                                                              DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                                                              IID_PPV_ARGS(&adapter));
            if (FAILED(hr))
                break;

            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) // Don't select the Basic Render Driver adapter.
            {
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
				return adapter;
            }
        }
    }

    if (adapter.Get() == nullptr)
    {
		for (u32 adapterIndex = 0; ; adapterIndex++)
		{
			HRESULT hr = m_DXGIFactory->EnumAdapters1(adapterIndex, &adapter);
			if (FAILED(hr))
				break;

			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) //We want hardware adapter not software - use warp for software
			{
				continue;
			}

			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) // Check to see whether the adapter supports Direct3D 12, but don't create the actual device yet.
			{
				return adapter;
			}
		}
    }

	return nullptr;
}
