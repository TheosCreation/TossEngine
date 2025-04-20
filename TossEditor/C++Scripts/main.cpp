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

#include <windows.h>
#include <TossEngine.h>

extern "C" TOSSENGINE_API void SetImGuiContext(ImGuiContext* context);

void SetImGuiContext(ImGuiContext* context)
{
    ImGui::SetCurrentContext(context);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        ComponentRegistry::GetInstance().CleanUpModule((void*)hModule);
        break;
    }
    return TRUE;
}