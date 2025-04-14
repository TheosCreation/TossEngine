#include "ScriptLoader.h"

ScriptLoader::ScriptLoader() : scriptsDll(nullptr) 
{
}

ScriptLoader::~ScriptLoader()
{
}

void ScriptLoader::reloadDLL()
{
    unloadDLL();
    CompileScriptsProject();
    loadDLL();
}

typedef void (*SetImGuiContextFunc)(ImGuiContext*);

void ScriptLoader::loadDLL()
{
    scriptsDll = LoadLibrary(L"C++Scripts.dll");
    if (scriptsDll)
    {
        Debug::Log("Scripts DLL loaded.");

        SetImGuiContextFunc setImGuiContext = reinterpret_cast<SetImGuiContextFunc>(
            GetProcAddress(scriptsDll, "SetImGuiContext")
            );
        if (setImGuiContext)
        {
            // Pass the host application's current ImGui context
            ImGuiContext* mainContext = ImGui::GetCurrentContext();
            setImGuiContext(mainContext);
            Debug::Log("ImGui context set in DLL.");
        }
        else
        {
            DWORD errorCode = GetLastError();
            string errorMessage = "Failed to find SetImGuiContext. Win32 Error Code: " + ToString(errorCode);
            Debug::LogError(errorMessage, false);
        }
    }
    else
    {
        DWORD errorCode = GetLastError();
        string errorMessage = "Failed to load C++Scripts.dll. Win32 Error Code: " + ToString(errorCode);
        Debug::LogError(errorMessage, false);
    }
}

void ScriptLoader::CompileScriptsProject()
{
    std::string msBuildPath = getMSBuildPath();
    std::string solutionPath = FindSolutionPath(); //this needs to change to proper name of the solution on the release build of the engine
    std::string config;

    #ifdef _DEBUG
        config = "Debug";
    #else
        config = "Release";
    #endif

    if (msBuildPath.empty()) {
        Debug::LogError("No buildpath (MSBuild.exe not found) | Did not recompile scripts", false);
        return;
    }

    if (solutionPath.empty()) {
        Debug::LogError("Could not locate TossEngine.sln | Did not recompile scripts", false);
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
        Debug::LogWarning("Scripts Project compilation failed.");
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