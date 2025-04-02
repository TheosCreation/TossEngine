#pragma once

#include "../PhysicsMaterial.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_15218338127896916913u = PhysicsMaterial::staticGetArchetype(); }

rfk::Class const& PhysicsMaterial::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("PhysicsMaterial", 15218338127896916913u, sizeof(PhysicsMaterial), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Resource>(), static_cast<rfk::EAccessSpecifier>(1));
PhysicsMaterial::_rfk_registerChildClass<PhysicsMaterial>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<PhysicsMaterial>>(),new rfk::NonMemberFunction<rfk::SharedPtr<PhysicsMaterial>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<PhysicsMaterial>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<PhysicsMaterial>>(),new rfk::NonMemberFunction<rfk::UniquePtr<PhysicsMaterial>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<PhysicsMaterial>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<PhysicsMaterial>() noexcept { return &PhysicsMaterial::staticGetArchetype(); }


