#pragma once

#include "../TerrainGameObject.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_14282517910864448885u = TerrainGameObject::staticGetArchetype(); }

rfk::Class const& TerrainGameObject::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("TerrainGameObject", 14282517910864448885u, sizeof(TerrainGameObject), 1);
if (!initialized) {
initialized = true;
TerrainGameObject::_rfk_registerChildClass<TerrainGameObject>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<TerrainGameObject>>(),new rfk::NonMemberFunction<rfk::SharedPtr<TerrainGameObject>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<TerrainGameObject>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<TerrainGameObject>>(),new rfk::NonMemberFunction<rfk::UniquePtr<TerrainGameObject>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<TerrainGameObject>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<TerrainGameObject>() noexcept { return &TerrainGameObject::staticGetArchetype(); }


