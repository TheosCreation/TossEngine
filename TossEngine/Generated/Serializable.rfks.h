#pragma once

#include "../Serializable.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_18162910670657723080u = Serializable::staticGetArchetype(); }

rfk::Class const& Serializable::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Serializable", 18162910670657723080u, sizeof(Serializable), 1);
if (!initialized) {
initialized = true;
Serializable::_rfk_registerChildClass<Serializable>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Serializable>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Serializable>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Serializable>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Serializable>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Serializable>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Serializable>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Serializable>() noexcept { return &Serializable::staticGetArchetype(); }


