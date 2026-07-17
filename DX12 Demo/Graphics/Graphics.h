#pragma once
#include "pch.h"
#include "CommandQueue.h"
#include "BufferAllocator.h"

struct Vertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Color;
};

class Graphics
{
public:
	void Initialize(bool useWarp = false);
	static Graphics* GetInstance();
	static CommandQueue* GetDirectCommandQueue();
	static CommandQueue* GetTransferCommandQueue();

	static ID3D12Device2* GetDevice();
	static IDXGIFactory4* GetDXGIFactory();
	static D3D_ROOT_SIGNATURE_VERSION GetHighestRootSignatureVersion();
	static bool IsTearingSupported();
	static bool IsVSyncOn();
private:
	void EnableDebugLayer();
	void InitializeDXGIFactory();
	void InitializeDevice();
	ComPtr<IDXGIAdapter1> GetHardwareAdapter();

	bool m_UseWarpDevice = false;
	ComPtr<IDXGIFactory5> m_DXGIFactory;
	ComPtr<ID3D12Device2> m_Device;
	ComPtr<D3D12MA::Allocator> m_Allocator = nullptr;
	std::unique_ptr<CommandQueue> m_DirectCommandQueue;
	std::unique_ptr<CommandQueue> m_TransferCommandQueue;

	D3D_ROOT_SIGNATURE_VERSION m_HighestRootSignatureVersion;
	u32 m_RTVDescriptorSize = 0;
	u32 m_CurrentBackBufferIndex = 0;
	bool m_TearingSupported = false;
	bool m_DebugLayerEnabled = true;
	bool m_Vsync = true;
};