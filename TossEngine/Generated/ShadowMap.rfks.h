#pragma once

#include "../ShadowMap.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_692139092585123571u = ShadowMap::staticGetArchetype(); }

rfk::Class const& ShadowMap::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("ShadowMap", 692139092585123571u, sizeof(ShadowMap), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Texture>(), static_cast<rfk::EAccessSpecifier>(1));
ShadowMap::_rfk_registerChildClass<ShadowMap>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<ShadowMap>>(),new rfk::NonMemberFunction<rfk::SharedPtr<ShadowMap>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<ShadowMap>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<ShadowMap>>(),new rfk::NonMemberFunction<rfk::UniquePtr<ShadowMap>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<ShadowMap>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<ShadowMap>() noexcept { return &ShadowMap::staticGetArchetype(); }


