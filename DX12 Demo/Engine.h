#pragma once
#include "pch.h"
#include "Graphics/Renderer.h"
#include "../Window/Window.h"

class Engine
{
public:
	bool Initialize();
	void OnUpdate();
	void OnRender();
	void ProcessWindowsMessages();
	bool IsRunning() const;
private:
	Window m_Window;
	Renderer m_Renderer;
	bool m_IsRunning = true;
};