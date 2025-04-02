#pragma once

#include "../Resizable.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_8542059403700411124u = Resizable::staticGetArchetype(); }

rfk::Class const& Resizable::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Resizable", 8542059403700411124u, sizeof(Resizable), 1);
if (!initialized) {
initialized = true;
Resizable::_rfk_registerChildClass<Resizable>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Resizable>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Resizable>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Resizable>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Resizable>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Resizable>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Resizable>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Resizable>() noexcept { return &Resizable::staticGetArchetype(); }


