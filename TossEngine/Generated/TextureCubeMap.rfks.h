#pragma once

#include "../TextureCubeMap.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_10475015047235674639u = TextureCubeMap::staticGetArchetype(); }

rfk::Class const& TextureCubeMap::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("TextureCubeMap", 10475015047235674639u, sizeof(TextureCubeMap), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Texture>(), static_cast<rfk::EAccessSpecifier>(1));
TextureCubeMap::_rfk_registerChildClass<TextureCubeMap>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<TextureCubeMap>>(),new rfk::NonMemberFunction<rfk::SharedPtr<TextureCubeMap>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<TextureCubeMap>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<TextureCubeMap>>(),new rfk::NonMemberFunction<rfk::UniquePtr<TextureCubeMap>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<TextureCubeMap>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<TextureCubeMap>() noexcept { return &TextureCubeMap::staticGetArchetype(); }


