#pragma once
#include "Utils.h"
#include <Windows.h>

class TOSSENGINE_API ScriptLoader {
public:
    ScriptLoader();
    ~ScriptLoader();

    void reloadDLL();

private:
    void loadDLL();
    void CompileScriptsProject();

    void unloadDLL();

    HMODULE scriptsDll;
};