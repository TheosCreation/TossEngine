#pragma once

#include "../GameObject.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_563616575858621630u = GameObject::staticGetArchetype(); }

rfk::Class const& GameObject::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("GameObject", 563616575858621630u, sizeof(GameObject), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(2);
type.addDirectParent(rfk::getArchetype<Serializable>(), static_cast<rfk::EAccessSpecifier>(1));
type.addDirectParent(rfk::getArchetype<ISelectable>(), static_cast<rfk::EAccessSpecifier>(1));
GameObject::_rfk_registerChildClass<GameObject>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<GameObject>>(),new rfk::NonMemberFunction<rfk::SharedPtr<GameObject>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<GameObject>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<GameObject>>(),new rfk::NonMemberFunction<rfk::UniquePtr<GameObject>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<GameObject>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<GameObject>() noexcept { return &GameObject::staticGetArchetype(); }


