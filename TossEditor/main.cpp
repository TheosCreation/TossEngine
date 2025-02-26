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

#include "Game.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>

void InitMono()
{
    // Get Solution Directory
    std::string solutionDir = std::filesystem::current_path().parent_path().string();

    // Set Mono directories
    mono_set_dirs((solutionDir + "/Dependencies/MONO/lib").c_str(), (solutionDir + "/Dependencies/MONO/etc").c_str());

    // Initialize Mono
    mono_config_parse(NULL);
    MonoDomain* domain = mono_jit_init("TossEngine");
    if (!domain)
    {
        std::cerr << "Failed to initialize Mono JIT." << std::endl;
        return;
    }

    // Load the C# Assembly (Scripts.dll)
    std::string scriptDllPath = solutionDir + "/Dependencies/MONO/CompliedScriptDLLs/Scripts.dll";
    MonoAssembly* assembly = mono_domain_assembly_open(domain, scriptDllPath.c_str());
    MonoImage* image = mono_assembly_get_image(assembly);
    MonoClass* shipClass = mono_class_from_name(image, "Scripts", "Ship");
    MonoMethod* logSomethingMethod = mono_class_get_method_from_name(shipClass, "LogSomething", 0);
    MonoObject* shipObject = mono_object_new(domain, shipClass);

    mono_runtime_object_init(shipObject); // Call Constructor

    // Invoke LogSomething() on Ship Instance
    mono_runtime_invoke(logSomethingMethod, shipObject, NULL, NULL);

    // Cleanup Mono when the application exits
    mono_jit_cleanup(domain);
}

int main()
{
    //Init Mono
    InitMono();

    try
    {
        Game game; // Create an instance of the game
        game.run();  // Run the game
    }
    catch (const std::exception& e)
    {
        std::wclog << e.what() << std::endl; // Log any exceptions that occur
        return -1; // Return -1 to indicate an error
    }

    return 0; // Return 0 to indicate successful execution
}