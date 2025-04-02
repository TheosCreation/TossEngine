#pragma once

#include "../Ship.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_15608329644900673865u = Ship::staticGetArchetype(); }

rfk::Class const& Ship::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Ship", 15608329644900673865u, sizeof(Ship), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Component>(), static_cast<rfk::EAccessSpecifier>(1));
Ship::_rfk_registerChildClass<Ship>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Ship>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Ship>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Ship>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Ship>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Ship>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Ship>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Ship>() noexcept { return &Ship::staticGetArchetype(); }


