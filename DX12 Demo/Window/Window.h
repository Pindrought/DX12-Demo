#pragma once
#include "pch.h"
#include "WindowCreationParameters.h"

struct WindowSettings
{
	bool IsClosed = false;
	WindowStyle Style = WindowStyle::ExitButton;
	u32 Width = 0;
	u32 Height = 0;
	std::string Title = "";
	bool Enabled = true;
	bool Visible = true;
	DWORD WINAPIWindowStyle = 0;
	DWORD WINAPIExtendedWindowStyle = 0;
	std::string WindowClassName = "";
};

class Window
{
public:
	bool Initialize(WindowCreationParameters params);
	~Window();

	RECT GetClientRect() const;
	void SetEnabled(bool enabled);
	void Hide();
	void Show();

	void RegisterWindowClass();
	LRESULT WindowProcA(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void InitializeSwapChain();
	ComPtr<IDXGISwapChain4> m_SwapChain = nullptr;
public:
	WindowSettings m_Settings;
public:
	HWND m_HWND = NULL;
	ComPtr<ID3D12DescriptorHeap> m_RTVHeap = nullptr;
	ComPtr<ID3D12Resource> m_BackBuffer[NUMBER_FRAMES_IN_FLIGHT];
	ComPtr<ID3D12Resource> m_RenderTargets[NUMBER_FRAMES_IN_FLIGHT];
	u32 m_RTVDescriptorSize = 0;
	u32 m_FrameIndex = 0;
};