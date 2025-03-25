#include "ScriptLoader.h"

ScriptLoader::ScriptLoader() : scriptsDll(nullptr) 
{
    loadDLL();
}

ScriptLoader::~ScriptLoader()
{
    unloadDLL();
}

void ScriptLoader::reloadDLL()
{
    unloadDLL();
    CompileScriptsProject();
    loadDLL();
}

void ScriptLoader::loadDLL()
{
    scriptsDll = LoadLibrary(L"C++Scripts.dll");
    if (scriptsDll)
    {
        Debug::Log("Scripts DLL loaded.");

        typedef void (*RegisterComponentsFunc)();
        RegisterComponentsFunc registerFunc = (RegisterComponentsFunc)GetProcAddress(scriptsDll, "RegisterComponents");
        if (registerFunc) {
            registerFunc();
            Debug::Log("Register function called.");
        }
    }
    else
    {
        Debug::Log("No Scripts dll found.");
    }
}

void ScriptLoader::CompileScriptsProject()
{
    const char* command = R"(cmd /C ""C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" "C:\Users\Theo\TossEngine\TossEngine.sln" /t:C++Scripts /p:Configuration=Release /p:Platform=x64")";
    int result = system(command);

    if (result == 0) {
        Debug::Log("Scripts Project compiled successfully.");
    }
    else {
        Debug::LogError("Scripts Project compilation failed.");
    }
}

void ScriptLoader::unloadDLL()
{
    if (scriptsDll) {
        FreeLibrary(scriptsDll);
        scriptsDll = nullptr;
        Debug::Log("Scripts DLL unloaded.");
    }
}