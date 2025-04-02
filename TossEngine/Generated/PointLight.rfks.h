#pragma once

#include "../PointLight.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_5962282563482002885u = PointLight::staticGetArchetype(); }

rfk::Class const& PointLight::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("PointLight", 5962282563482002885u, sizeof(PointLight), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Component>(), static_cast<rfk::EAccessSpecifier>(1));
PointLight::_rfk_registerChildClass<PointLight>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<PointLight>>(),new rfk::NonMemberFunction<rfk::SharedPtr<PointLight>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<PointLight>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<PointLight>>(),new rfk::NonMemberFunction<rfk::UniquePtr<PointLight>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<PointLight>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<PointLight>() noexcept { return &PointLight::staticGetArchetype(); }


