#pragma once

#include "../Component.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_6949090008875821326u = Component::staticGetArchetype(); }

rfk::Class const& Component::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Component", 6949090008875821326u, sizeof(Component), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(2);
type.addDirectParent(rfk::getArchetype<Serializable>(), static_cast<rfk::EAccessSpecifier>(1));
type.addDirectParent(rfk::getArchetype<ISelectable>(), static_cast<rfk::EAccessSpecifier>(1));
Component::_rfk_registerChildClass<Component>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Component>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Component>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Component>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Component>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Component>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Component>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Component>() noexcept { return &Component::staticGetArchetype(); }


