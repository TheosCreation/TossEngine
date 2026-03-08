/***
DeviousDevs
Auckland
New Zealand
(c) 2026 DeviousDevs
File Name : main.cpp
Description : The entry point of the library
Author : Theo Morris
Mail : theo.morris@outlook.co.nz
**/

#include <windows.h>
#include "TossEngine.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}