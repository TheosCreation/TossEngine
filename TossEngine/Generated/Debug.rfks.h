#pragma once

#include "../Debug.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_7216566150432796378u = Debug::staticGetArchetype(); }

rfk::Class const& Debug::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Debug", 7216566150432796378u, sizeof(Debug), 1);
if (!initialized) {
initialized = true;
Debug::_rfk_registerChildClass<Debug>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Debug>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Debug>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Debug>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Debug>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Debug>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Debug>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Debug>() noexcept { return &Debug::staticGetArchetype(); }


