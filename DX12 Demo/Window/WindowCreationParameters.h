#pragma once
#include "pch.h"
#include "WindowStyle.h"

struct WindowCreationParameters
{
	uint16_t Width = 800;
	uint16_t Height = 600;
	std::string Title = "Window Title";
	std::string ClassName = "EngineClassName";
	WindowStyle Style = WindowStyle::ExitButton;
	int XPosition = INT_MAX;
	int YPosition = INT_MAX;
	uint8_t SampleCount = 8;
	HWND ParentWindow = NULL;
};