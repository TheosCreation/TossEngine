#pragma once
#pragma warning(disable : 4251)

#ifdef TOSSENGINE_EXPORTS
#define TOSSENGINE_API __declspec(dllexport)  // When compiling the DLL
#else
#define TOSSENGINE_API __declspec(dllimport)  // When using the DLL
#endif