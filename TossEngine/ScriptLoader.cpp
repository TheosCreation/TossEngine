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
    std::string solutionPath = FindSolutionPath("TossEngine.sln"); //this needs to change to proper name of the solution on the release build of the engine

    if (msBuildPath.empty()) {
        Debug::LogError("No buildpath (MSBuild.exe not found)");
        return;
    }

    if (solutionPath.empty()) {
        Debug::LogError("Could not locate TossEngine.sln");
        return;
    }

    std::string command = std::string("cmd /C \"\"") + msBuildPath +
        "\" \"" + solutionPath + "\" /t:C++Scripts /p:Configuration=Release /p:Platform=x64\"";

    int result = system(command.c_str());

    if (result == 0) {
        Debug::Log("Scripts Project compiled successfully.");
    }
    else {
        Debug::LogError("Scripts Project compilation failed.");
    }
}

std::string ScriptLoader::FindSolutionPath(const std::string& solutionName) {
    fs::path currentPath = fs::current_path();

    while (!currentPath.empty()) {
        fs::path solutionPath = currentPath / solutionName;
        if (fs::exists(solutionPath)) {
            return solutionPath.string();
        }
        currentPath = currentPath.parent_path();
    }
    return "";  // Not found
}

string ScriptLoader::getMSBuildPath() {
    const char* vswhereCmd =
        R"("C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe)";

    std::array<char, 512> buffer;
    std::string result;

    // Use pipe to execute command and get output
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(vswhereCmd, "r"), _pclose);
    if (!pipe) {
        throw std::runtime_error("Failed to run vswhere.");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    // Remove potential trailing newline
    result.erase(result.find_last_not_of(" \n\r\t") + 1);
    return result;
}

void ScriptLoader::unloadDLL()
{
    if (scriptsDll) {
        FreeLibrary(scriptsDll);
        scriptsDll = nullptr;
        Debug::Log("Scripts DLL unloaded.");
    }
}