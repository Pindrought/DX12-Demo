#include "pch.h"
#include "Window.h"
#include "../Graphics/Graphics.h"

bool Window::Initialize(WindowCreationParameters params)
{
	DBG_LOG("[Window::Initialize()]");

	m_Settings.Width = params.Width;
	m_Settings.Height = params.Height;
	m_Settings.Title = params.Title;
	m_Settings.WindowClassName = params.ClassName;
	m_Settings.Style = params.Style;

	static bool raw_input_initialized = false;
	if (raw_input_initialized == false)
	{
		RAWINPUTDEVICE rid;

		rid.usUsagePage = 0x01; //Mouse
		rid.usUsage = 0x02;
		rid.dwFlags = 0;
		rid.hwndTarget = NULL;

		if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
		{
			return false;
		}

		raw_input_initialized = true;
	}

	//Determine window x/y position
	if (params.XPosition == INT_MAX) //if no xpos entered, set to center of screen
	{
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int halfScreenWidth = screenWidth / 2;
		int halfWindowWidth = params.Width / 2;
		int centerScreenX = halfScreenWidth - halfWindowWidth;
		params.XPosition = centerScreenX;
	}

	if (params.YPosition == INT_MAX) //if no ypos entered, set to center of screen
	{
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		int halfScreenHeight = screenHeight / 2;
		int halfWindowHeight = params.Height / 2;
		int centerScreenY = halfScreenHeight - halfWindowHeight;
		params.YPosition = centerScreenY;
	}
	//Determine Window Rect
	RECT wr; //Window Rectangle
	wr.left = params.XPosition;
	wr.top = params.YPosition;
	wr.right = wr.left + params.Width;
	wr.bottom = wr.top + params.Height;
	RECT tempwr = wr;

	m_Settings.WINAPIWindowStyle = 0;
	m_Settings.WINAPIWindowStyle |= WS_POPUP;

	if (params.Style & WindowStyle::ExitButton)
	{
		m_Settings.WINAPIWindowStyle |= WS_SYSMENU | WS_CAPTION;
	}
	if (params.Style & WindowStyle::MinimizeAvailable)
	{
		m_Settings.WINAPIWindowStyle |= WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
	}
	if (params.Style & WindowStyle::MaximizeAvailable)
	{
		m_Settings.WINAPIWindowStyle |= WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX;
	}
	if (params.Style & WindowStyle::Resizable)
	{
		m_Settings.WINAPIWindowStyle |= WS_SIZEBOX;
	}
	if (params.Style & WindowStyle::NoBorder)
	{
		UINT styleFix = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
		styleFix = ~styleFix;
		m_Settings.WINAPIWindowStyle &= styleFix;
	}

	BOOL result = AdjustWindowRect(&wr, m_Settings.WINAPIWindowStyle, FALSE);
	if (result == 0) //If adjustwindowrect fails...
		return false;

	//Register Window Class
	m_Settings.WINAPIExtendedWindowStyle = 0;
	if (params.Style & WindowStyle::TransparencyAllowed)
	{
		m_Settings.WINAPIExtendedWindowStyle |= WS_EX_LAYERED;
	}
	if (params.Style & WindowStyle::Topmost)
	{
		m_Settings.WINAPIExtendedWindowStyle |= WS_EX_TOPMOST;
	}

	if (m_HWND != NULL)
	{
		DestroyWindow(m_HWND);
		UnregisterClassA(m_Settings.WindowClassName.c_str(), GetModuleHandle(NULL));
		m_HWND = NULL;
	}

	RegisterWindowClass();

	//Create Window
	m_HWND = CreateWindowExA(m_Settings.WINAPIExtendedWindowStyle, //Extended Windows style. For other options, see: https://msdn.microsoft.com/en-us/library/windows/desktop/ff700543(v=vs.85).aspx
							 m_Settings.WindowClassName.c_str(), //Window class name
							 m_Settings.Title.c_str(), //Window Title
							 m_Settings.WINAPIWindowStyle, //Windows style - See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms632600(v=vs.85).aspx
							 wr.left, //Window X Position
							 wr.top, //Window Y Position
							 wr.right - wr.left, //Window Width
							 wr.bottom - wr.top, //Window Height
							 NULL, //Handle to parent of this window. Since this is the first window, it has no parent window.
							 NULL, //Handle to menu or child window identifier. Can be set to NULL and use menu in WindowClassEx if a menu is desired to be used.
							 GetModuleHandle(NULL), //Handle to the instance of module to be used with this window
							 this); //Parameter passed to create window 'WM_NCCREATE'

	if (m_HWND == NULL)
	{
		DWORD error = GetLastError();
		return false;
	}

	InitializeSwapChain();

	DBG_LOG(sfmt("[Window::Initialize()] -> HWND [%d]", m_HWND));

	//Show/focus Window
	ShowWindow(m_HWND, SW_SHOW);
	SetForegroundWindow(m_HWND);
	SetFocus(m_HWND);

	return true;
}

Window::~Window()
{
	DBG_LOG(sfmt("[Window::~Window() - HWND [%d]", m_HWND));
	BOOL result = UnregisterClassA(m_Settings.WindowClassName.c_str(), GetModuleHandle(NULL));
	if (result == FALSE)
	{
		DBG_LOG(sfmt("[Window::~Window() - HWND [%d] failed to unregister window class.", m_HWND));
	}
}

RECT Window::GetClientRect() const
{
	RECT rect = { 0 };
	GetWindowRect(m_HWND, &rect);
	return rect;
}

void Window::SetEnabled(bool enabled)
{
	m_Settings.Enabled = enabled;
	if (enabled)
		EnableWindow(m_HWND, TRUE);
	else
		EnableWindow(m_HWND, FALSE);
}

void Window::Hide()
{
	m_Settings.Visible = false;
	ShowWindow(m_HWND, SW_HIDE);
}

void Window::Show()
{
	m_Settings.Visible = true;
	ShowWindow(m_HWND, SW_SHOW);
}

LRESULT Window::WindowProcA(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
	{
		m_Settings.IsClosed = true;
		return 0;
	}
	default:
		return DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}
}

void Window::InitializeSwapChain()
{
	DBG_LOG(sfmt("[Window::InitializeSwapChain()] -> HWND [%d]", m_HWND));

	auto pFactory = Graphics::GetDXGIFactory();
	auto pDevice = Graphics::GetDevice();


	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = NUMBER_FRAMES_IN_FLIGHT;
	swapChainDesc.Width = m_Settings.Width;
	swapChainDesc.Height = m_Settings.Height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	if (Graphics::IsTearingSupported())
	{
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	}

	ComPtr<IDXGISwapChain1> swapChain1; //CreateSwapChainForHwnd returns a swapchain that we will later cast to IDXGISwapChain4
	ThrowIfFailed(pFactory->CreateSwapChainForHwnd(Graphics::GetDirectCommandQueue()->GetD3D12CommandQueue(), // Swap chain needs the queue so that it can force a flush on it.
													m_HWND,
													&swapChainDesc,
													nullptr,
													nullptr,
													&swapChain1));

	ThrowIfFailed(swapChain1.As(&m_SwapChain)); //Cast it to IDXGISwapChain4

	ThrowIfFailed(pFactory->MakeWindowAssociation(m_HWND, DXGI_MWA_NO_ALT_ENTER));

	m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = NUMBER_FRAMES_IN_FLIGHT;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RTVHeap)));

		m_RTVDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < NUMBER_FRAMES_IN_FLIGHT; n++)
		{
			ThrowIfFailed(m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTargets[n])));
			pDevice->CreateRenderTargetView(m_RenderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_RTVDescriptorSize);
		}
	}

}

LRESULT CALLBACK HandleMsgRedirect(HWND hwnd,
								   UINT uMsg,
								   WPARAM wParam,
								   LPARAM lParam)
{
	switch (uMsg)
	{
		/*case WM_CLOSE:
			DestroyWindow(hwnd);
			return 0;*/

	default:
	{
		// retrieve ptr to window class
		Window* const pWindow = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		// forward message to window class handler
		return pWindow->WindowProcA(hwnd, uMsg, wParam, lParam);
	}
	}
}

LRESULT CALLBACK HandleMessageSetup(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NCCREATE:
	{
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* pWindow = reinterpret_cast<Window*>(pCreate->lpCreateParams);
		if (pWindow == nullptr) //Sanity check
		{
			exit(-1);
		}
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HandleMsgRedirect));
		return pWindow->WindowProcA(hwnd, uMsg, wParam, lParam);
	}
	default:
		return DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}
}

void Window::RegisterWindowClass()
{
	WNDCLASSEXA wc = {}; //Our Window Class (This has to be filled before our window can be created) See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633577(v=vs.85).aspx
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; //Flags [Redraw on width/height change from resize/movement] See: https://msdn.microsoft.com/en-us/library/windows/desktop/ff729176(v=vs.85).aspx
	wc.lpfnWndProc = HandleMessageSetup; //Pointer to Window Proc function for handling messages from this window
	wc.cbClsExtra = 0; //# of extra bytes to allocate following the window-class structure. We are not currently using this.
	wc.cbWndExtra = 0; //# of extra bytes to allocate following the window instance. We are not currently using this.
	wc.hInstance = GetModuleHandle(NULL); //Handle to the instance that contains the Window Procedure
	wc.hIcon = NULL;   //Handle to the class icon. Must be a handle to an icon resource. We are not currently assigning an icon, so this is null.
	wc.hIconSm = NULL; //Handle to small icon for this class. We are not currently assigning an icon, so this is null.
	wc.hCursor = NULL; //Default Cursor - If we leave this null, we have to explicitly set the cursor's shape each time it enters the window.
	wc.hbrBackground = NULL; //Handle to the class background brush for the window's background color - we will leave this blank for now and later set this to black. For stock brushes, see: https://msdn.microsoft.com/en-us/library/windows/desktop/dd144925(v=vs.85).aspx
	wc.lpszMenuName = NULL; //Pointer to a null terminated character string for the menu. We are not using a menu yet, so this will be NULL.
	wc.lpszClassName = m_Settings.WindowClassName.c_str(); //Pointer to null terminated string of our class name for this window.
	wc.cbSize = sizeof(WNDCLASSEXA); //Need to fill in the size of our struct for cbSize
	RegisterClassExA(&wc); // Register the class so that it is usable.
}