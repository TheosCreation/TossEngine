#pragma once
#include "Utils.h"
#include <Windows.h>

class TOSSENGINE_API ScriptLoader {
public:
    ScriptLoader();
    ~ScriptLoader();

    void reloadDLL();

    void unloadDLL();
    void loadDLL();
    void CompileScriptsProject();

private:

    HMODULE scriptsDll;
};