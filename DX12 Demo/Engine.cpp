#include "pch.h"
#include "Engine.h"
#include "../Window/Window.h"

bool Engine::Initialize()
{
#if defined(_DEBUG) ||defined(CONSOLE_LOG_ENABLED)
	//Create console for debug output
	AllocConsole();

	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
	freopen_s(&f, "CONOUT$", "w", stderr);
	freopen_s(&f, "CONIN$", "r", stdin);
#endif
	DBG_LOG("[Engine::Initialize()]");

	if (m_Renderer.Initialize() == false)
	{
		return false;
	}

	m_Window.Initialize(WindowCreationParameters{ 800, 600, "DX12 Demo", "DX12DemoClass", WindowStyle::ExitButton | WindowStyle::Resizable });


	return true;
}

void Engine::OnUpdate()
{
}

void Engine::OnRender()
{
	//OnPreRender

	m_Renderer.Render(&m_Window);
}

void Engine::ProcessWindowsMessages()
{
	MSG msg;
	while (PeekMessage(&msg, //Where to store message (if one exists) See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms644943(v=vs.85).aspx
					   NULL, //Handle to window we are checking messages for (NULL = all messages)
					   0,    //Minimum Filter Msg Value - We are not filtering for specific messages, but the min/max could be used to filter only mouse messages for example.
					   0,    //Maximum Filter Msg Value
					   PM_REMOVE))//Remove message after capturing it via PeekMessage. For more argument options, see: https://msdn.microsoft.com/en-us/library/windows/desktop/ms644943(v=vs.85).aspx
	{
		TranslateMessage(&msg); //Translate message from virtual key messages into character messages so we can dispatch the message. See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms644955(v=vs.85).aspx
		DispatchMessage(&msg); //Dispatch message to our Window Proc for this window. See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms644934(v=vs.85).aspx
	}
}

bool Engine::IsRunning() const
{
	return m_IsRunning && !m_Window.m_Settings.IsClosed;
}
