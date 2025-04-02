#pragma once

#include "../Spotlight.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_3533788579765571877u = Spotlight::staticGetArchetype(); }

rfk::Class const& Spotlight::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Spotlight", 3533788579765571877u, sizeof(Spotlight), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Component>(), static_cast<rfk::EAccessSpecifier>(1));
Spotlight::_rfk_registerChildClass<Spotlight>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Spotlight>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Spotlight>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Spotlight>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Spotlight>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Spotlight>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Spotlight>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Spotlight>() noexcept { return &Spotlight::staticGetArchetype(); }


