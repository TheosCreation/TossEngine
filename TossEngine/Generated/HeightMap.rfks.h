#pragma once

#include "../HeightMap.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_3705724552000127266u = HeightMap::staticGetArchetype(); }

rfk::Class const& HeightMap::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("HeightMap", 3705724552000127266u, sizeof(HeightMap), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Resource>(), static_cast<rfk::EAccessSpecifier>(1));
HeightMap::_rfk_registerChildClass<HeightMap>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<HeightMap>>(),new rfk::NonMemberFunction<rfk::SharedPtr<HeightMap>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<HeightMap>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<HeightMap>>(),new rfk::NonMemberFunction<rfk::UniquePtr<HeightMap>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<HeightMap>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<HeightMap>() noexcept { return &HeightMap::staticGetArchetype(); }


