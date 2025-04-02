#pragma once

#include "../Rigidbody.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_6101488944037903354u = Rigidbody::staticGetArchetype(); }

rfk::Class const& Rigidbody::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Rigidbody", 6101488944037903354u, sizeof(Rigidbody), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Component>(), static_cast<rfk::EAccessSpecifier>(1));
Rigidbody::_rfk_registerChildClass<Rigidbody>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Rigidbody>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Rigidbody>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Rigidbody>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Rigidbody>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Rigidbody>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Rigidbody>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Rigidbody>() noexcept { return &Rigidbody::staticGetArchetype(); }


