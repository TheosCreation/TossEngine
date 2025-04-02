#pragma once

#include "../Camera.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_3909919755325336564u = Camera::staticGetArchetype(); }

rfk::Class const& Camera::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Camera", 3909919755325336564u, sizeof(Camera), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Component>(), static_cast<rfk::EAccessSpecifier>(1));
Camera::_rfk_registerChildClass<Camera>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Camera>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Camera>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Camera>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Camera>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Camera>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Camera>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Camera>() noexcept { return &Camera::staticGetArchetype(); }


