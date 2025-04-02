#pragma once

#include "../ISelectable.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_5433989170712682778u = ISelectable::staticGetArchetype(); }

rfk::Class const& ISelectable::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("ISelectable", 5433989170712682778u, sizeof(ISelectable), 1);
if (!initialized) {
initialized = true;
ISelectable::_rfk_registerChildClass<ISelectable>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<ISelectable>>(),new rfk::NonMemberFunction<rfk::SharedPtr<ISelectable>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<ISelectable>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<ISelectable>>(),new rfk::NonMemberFunction<rfk::UniquePtr<ISelectable>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<ISelectable>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<ISelectable>() noexcept { return &ISelectable::staticGetArchetype(); }


