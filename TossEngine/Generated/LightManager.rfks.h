#pragma once

#include "../LightManager.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_7587163098735739124u = LightManager::staticGetArchetype(); }

rfk::Class const& LightManager::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("LightManager", 7587163098735739124u, sizeof(LightManager), 1);
if (!initialized) {
initialized = true;
LightManager::_rfk_registerChildClass<LightManager>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<LightManager>>(),new rfk::NonMemberFunction<rfk::SharedPtr<LightManager>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<LightManager>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<LightManager>>(),new rfk::NonMemberFunction<rfk::UniquePtr<LightManager>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<LightManager>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<LightManager>() noexcept { return &LightManager::staticGetArchetype(); }


