/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : ScriptLoader.h
Description : Handles dynamic loading, unloading, and compilation of the script DLL (e.g., GameComponents.dll).
              Supports hot-reloading workflow during editor runtime.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Utils.h"
#include <Windows.h>

/**
 * @class ScriptLoader
 * @brief Manages the lifecycle of the script DLL used in TossEngine.
 *        Supports manual compilation, loading, unloading, and hot-reloading.
 */
class TOSSENGINE_API ScriptLoader {
public:
    /**
     * @brief Constructor.
     *        Initializes internal state (does not load DLL).
     */
    ScriptLoader();

    /**
     * @brief Destructor.
     *        Ensures the DLL is unloaded on destruction.
     */
    ~ScriptLoader();

    /**
     * @brief Reloads the DLL by unloading and loading it again.
     */
    void reloadDLL();

    /**
     * @brief Unloads the currently loaded DLL.
     */
    void unloadDLL();

    /**
     * @brief Loads the DLL from disk.
     */
    void loadDLL();

    /**
     * @brief Compiles the script project (e.g., via MSBuild).
     *        Paths and build configurations should be defined in Utils or config.
     */
    void CompileScriptsProject();

private:
    /**
     * @brief Handle to the currently loaded DLL module.
     */
    HMODULE scriptsDll = nullptr;
};