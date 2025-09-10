// EngineInterface.cxx
#include "EngineInterface.hxx"
#include "../ScriptCore/Application.h"

using namespace System;
#pragma comment (lib, "ScriptCore.lib")

namespace ScriptAPI
{
    void EngineInterface::HelloWorld()
    {
        System::Console::WriteLine("Hello Managed World!");
        Core::Application::HelloWorld();
    }
}