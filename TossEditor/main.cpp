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

int main()
{
    try
    {
        TossEditor tossEditor; // Create an instance of the editor
        tossEditor.run();  // Run the editor
    }
    catch (const std::exception& e)
    {
        std::wclog << e.what() << std::endl; // Log any exceptions that occur
        return -1; // Return -1 to indicate an error
    }

    return 0; // Return 0 to indicate successful execution
}