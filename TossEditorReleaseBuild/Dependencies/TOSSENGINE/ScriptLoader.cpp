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
    std::string msBuildPath = getMSBuildPath();
    std::string solutionPath = FindSolutionPath("TossEditor.sln"); //this needs to change to proper name of the solution on the release build of the engine
    std::string config;

    #ifdef _DEBUG
        config = "Debug";
    #else
        config = "Release";
    #endif

    if (msBuildPath.empty()) {
        Debug::LogError("No buildpath (MSBuild.exe not found)");
        return;
    }

    if (solutionPath.empty()) {
        Debug::LogError("Could not locate TossEngine.sln");
        return;
    }

    std::string command = std::string("cmd /C \"\"") + msBuildPath +
        "\" \"" + solutionPath +
        "\" /t:C++Scripts /p:Configuration=" + config + " /p:Platform=x64\"";

    int result = system(command.c_str());

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