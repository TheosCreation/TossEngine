/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : main.cpp
Description : The entry point of the library
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "TossEngine.h"

#ifdef _WIN32
#ifndef APIENTRY
#define APIENTRY __stdcall
#endif

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef void* HMODULE;
typedef unsigned long DWORD;
typedef void* LPVOID;

extern "C" __declspec(dllexport)
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case 1: // DLL_PROCESS_ATTACH
        break;
    case 0: // DLL_PROCESS_DETACH
        break;
    }
    return TRUE;
}
#endif