#pragma once

#ifdef TOSSENGINE_EXPORTS
#define TOSSENGINE_API __declspec(dllexport)  // When compiling the DLL
#else
#define TOSSENGINE_API __declspec(dllimport)  // When using the DLL
#endif