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


#ifdef _DEBUG
int main()
{
    try
    {
        TossPlayer tossPlayer; // Create an instance of the player
        tossPlayer.run();  // Run the player
    }
    catch (const std::exception& e)
    {
        return -1; // Return -1 to indicate an error
    }

    return 0; // Return 0 to indicate successful execution
}
#else

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    try
    {
        TossPlayer tossPlayer; // Create an instance of the player
        tossPlayer.run();  // Run the player
    }
    catch (const std::exception& e)
    {
        return -1; // Return -1 to indicate an error
    }

    return 0; // Return 0 to indicate successful execution
}
#endif