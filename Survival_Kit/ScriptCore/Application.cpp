// Application.cpp
#include "Application.h"
#include <Windows.h>
namespace Core
{
    void Application::Run()
    {
        while (true)
        {
            if (GetKeyState(VK_ESCAPE) & 0x8000)
                break;
        }
    }
}