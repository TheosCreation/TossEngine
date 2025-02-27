#pragma once
#include "Utils.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

class Component;

class TOSSENGINE_API MonoIntegration {
public:
    static MonoDomain* monoDomain;
    static MonoAssembly* monoAssembly;
    static MonoImage* monoImage;

    static void InitializeMono();

    static Component* CreateCSharpComponent(const std::string& className);

    static void ShutdownMono();
};