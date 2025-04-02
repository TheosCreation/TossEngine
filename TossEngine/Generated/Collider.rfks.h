#pragma once

#include "../Collider.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_17409785000366770127u = Collider::staticGetArchetype(); }

rfk::Class const& Collider::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Collider", 17409785000366770127u, sizeof(Collider), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Component>(), static_cast<rfk::EAccessSpecifier>(1));
Collider::_rfk_registerChildClass<Collider>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Collider>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Collider>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Collider>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Collider>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Collider>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Collider>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Collider>() noexcept { return &Collider::staticGetArchetype(); }


