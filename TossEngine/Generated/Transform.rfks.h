#pragma once

#include "../Transform.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_11441756021884995653u = Transform::staticGetArchetype(); }

rfk::Struct const& Transform::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("Transform", 11441756021884995653u, sizeof(Transform), 0);
if (!initialized) {
initialized = true;
Transform::_rfk_registerChildClass<Transform>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Transform>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Transform>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Transform>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Transform>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Transform>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Transform>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Transform>() noexcept { return &Transform::staticGetArchetype(); }


