#include "MonoIntegration.h"
#include "Component.h"

// Define static members
MonoDomain* MonoIntegration::monoDomain = nullptr;
MonoAssembly* MonoIntegration::monoAssembly = nullptr;
MonoImage* MonoIntegration::monoImage = nullptr;

void MonoIntegration::InitializeMono()
{

    // Get Solution Directory
    std::string solutionDir = std::filesystem::current_path().parent_path().string();
    std::string scriptDllPath = solutionDir + "/Dependencies/MONO/CompliedScriptDLLs/Scripts.dll";

    mono_set_dirs((solutionDir + "/Dependencies/MONO/lib").c_str(), (solutionDir + "/Dependencies/MONO/etc").c_str());
    monoDomain = mono_jit_init("TossEngine");
    if (!monoDomain) {
        std::cerr << "Failed to initialize Mono JIT.\n";
        return;
    }

    monoAssembly = mono_domain_assembly_open(monoDomain, scriptDllPath.c_str());
    if (!monoAssembly) {
        std::cerr << "Failed to load assembly: " << "\n";
        return;
    }

    monoImage = mono_assembly_get_image(monoAssembly);;
    if (!monoImage) {
        std::cerr << "Failed to get Mono image from assembly.\n";
        return;
    }
    std::cout << "Mono initialized successfully" << "\n";
}

Component* MonoIntegration::CreateCSharpComponent(const std::string& className)
{
    MonoClass* monoClass = mono_class_from_name(monoImage, "Scripts", className.c_str());
    if (!monoClass)
    {
        std::cerr << "C# Component type '" << className << "' not found!\n";
        return nullptr;
    }

    // Create a new instance of the C# class
    MonoObject* csharpObject = mono_object_new(monoDomain, monoClass);
    mono_runtime_object_init(csharpObject);

    // Wrap it into a C++ Component
    Component* component = new Component();
    component->SetMonoObject(csharpObject); // Hypothetical function to link C++ and C# object

    return component;
}

void MonoIntegration::ShutdownMono()
{
    if (monoDomain) {
        mono_jit_cleanup(monoDomain);
        monoDomain = nullptr;
        monoAssembly = nullptr;
        monoImage = nullptr;
        std::cout << "Mono shutdown successfully.\n";
    }
}