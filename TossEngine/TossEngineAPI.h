/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : TossEngineAPI.h
Description : Defines the import/export macros used for building or linking the TossEngine DLL.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

// Disable warning C4251: "needs to have dll-interface" warning for STL types
#pragma warning(disable : 4251)

/**
 * @brief Defines the TOSSENGINE_API macro for DLL export/import.
 *
 * When TOSSENGINE_EXPORTS is defined (during DLL build), symbols are exported.
 * When TOSSENGINE_EXPORTS is not defined (when using the DLL), symbols are imported.
 */
#ifdef TOSSENGINE_EXPORTS
 // Exporting symbols (building the DLL)
#define TOSSENGINE_API __declspec(dllexport)
#else
 // Importing symbols (using the DLL)
#define TOSSENGINE_API __declspec(dllimport)
#endif