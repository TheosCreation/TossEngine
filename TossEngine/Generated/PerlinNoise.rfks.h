#pragma once

#include "../PerlinNoise.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_16248600581116585937u = PerlinNoise::staticGetArchetype(); }

rfk::Class const& PerlinNoise::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("PerlinNoise", 16248600581116585937u, sizeof(PerlinNoise), 1);
if (!initialized) {
initialized = true;
PerlinNoise::_rfk_registerChildClass<PerlinNoise>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<PerlinNoise>>(),new rfk::NonMemberFunction<rfk::SharedPtr<PerlinNoise>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<PerlinNoise>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<PerlinNoise>>(),new rfk::NonMemberFunction<rfk::UniquePtr<PerlinNoise>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<PerlinNoise>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<PerlinNoise>() noexcept { return &PerlinNoise::staticGetArchetype(); }


