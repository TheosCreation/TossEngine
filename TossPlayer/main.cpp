#include "TossPlayer.h"
#include <Windows.h>

/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : main.cpp
Description : The entry point of the application
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    try
    {
        TossPlayer tossEditor; // Create an instance of the executable game player
        tossEditor.run();  // Run the editor
    }
    catch (const std::exception& e)
    {
        MessageBoxA(NULL, e.what(), "Error", MB_ICONERROR | MB_OK); // Display error in a message box
        return -1; // Return -1 to indicate an error
    }

    return 0; // Return 0 to indicate successful execution
}