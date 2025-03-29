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
**/
#include "TossEditor.h"
#include <Windows.h>

#ifdef _DEBUG
int main()
{
    try
    {
        TossEditor tossEditor; // Create an instance of the editor
        tossEditor.run();  // Run the editor
    }
    catch (const std::exception& e)
    {
        std::wclog << L"Exception: " << e.what() << std::endl;
        std::wclog << L"Press Enter to close..." << std::endl;
        std::wcin.get(); // Wait for user input
        return -1; // Return -1 to indicate an error
    }

    return 0; // Return 0 to indicate successful execution
}
#else

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    try
    {
        TossEditor tossEditor; // Create an instance of the editor
        tossEditor.run();  // Run the editor
    }
    catch (const std::exception& e)
    {
        std::wclog << L"Exception: " << e.what() << std::endl;
        std::wclog << L"Press Enter to close..." << std::endl;
        std::wcin.get(); // Wait for user input
        return -1; // Return -1 to indicate an error
    }

    return 0; // Return 0 to indicate successful execution
}
#endif