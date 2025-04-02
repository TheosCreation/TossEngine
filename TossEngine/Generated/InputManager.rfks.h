#pragma once

#include "../InputManager.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_13367778270943227584u = InputManager::staticGetArchetype(); }

rfk::Class const& InputManager::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("InputManager", 13367778270943227584u, sizeof(InputManager), 1);
if (!initialized) {
initialized = true;
InputManager::_rfk_registerChildClass<InputManager>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<InputManager>>(),new rfk::NonMemberFunction<rfk::SharedPtr<InputManager>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<InputManager>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<InputManager>>(),new rfk::NonMemberFunction<rfk::UniquePtr<InputManager>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<InputManager>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<InputManager>() noexcept { return &InputManager::staticGetArchetype(); }


