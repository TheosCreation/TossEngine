#pragma once
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <iostream>
#include <string>
#include <filesystem>

class Component;

class MonoIntegration {
public:
    static MonoDomain* monoDomain;
    static MonoAssembly* monoAssembly;
    static MonoImage* monoImage;

    static void InitializeMono();

    static Component* CreateCSharpComponent(const std::string& className);

    static void ShutdownMono();
};