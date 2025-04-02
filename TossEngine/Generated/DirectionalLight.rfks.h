#pragma once

#include "../DirectionalLight.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_4221051550562211003u = DirectionalLight::staticGetArchetype(); }

rfk::Class const& DirectionalLight::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("DirectionalLight", 4221051550562211003u, sizeof(DirectionalLight), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Component>(), static_cast<rfk::EAccessSpecifier>(1));
DirectionalLight::_rfk_registerChildClass<DirectionalLight>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<DirectionalLight>>(),new rfk::NonMemberFunction<rfk::SharedPtr<DirectionalLight>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<DirectionalLight>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<DirectionalLight>>(),new rfk::NonMemberFunction<rfk::UniquePtr<DirectionalLight>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<DirectionalLight>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<DirectionalLight>() noexcept { return &DirectionalLight::staticGetArchetype(); }


