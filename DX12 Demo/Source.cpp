#include "pch.h"
#include "Engine.h"
#include "Window/Window.h"

int WINAPI wWinMain(_In_     HINSTANCE hInstance,
					_In_opt_ HINSTANCE hPrevInstance,
					_In_     PWSTR pCmdLine,
					_In_     int nCmdShow)
{
    Timer t;
    t.Start();
    float x = t.GetMilisecondsElapsed();
    try
    {
        Engine engine;
        if (engine.Initialize() == false)
        {
            return -1;
        }

        
        while (engine.IsRunning())
        {
            engine.ProcessWindowsMessages();
            engine.OnUpdate();
            engine.OnRender();
        }
    }
    catch (const std::exception& e)
    {
        DBG_LOG(e.what());
		MessageBoxA(NULL, e.what(), "Error", MB_OK | MB_ICONERROR);
        return -1;
	}

	return 0;
}