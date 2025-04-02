#pragma once

#include "../GameObjectManager.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_13598953390185447597u = GameObjectManager::staticGetArchetype(); }

rfk::Class const& GameObjectManager::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("GameObjectManager", 13598953390185447597u, sizeof(GameObjectManager), 1);
if (!initialized) {
initialized = true;
GameObjectManager::_rfk_registerChildClass<GameObjectManager>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<GameObjectManager>>(),new rfk::NonMemberFunction<rfk::SharedPtr<GameObjectManager>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<GameObjectManager>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<GameObjectManager>>(),new rfk::NonMemberFunction<rfk::UniquePtr<GameObjectManager>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<GameObjectManager>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<GameObjectManager>() noexcept { return &GameObjectManager::staticGetArchetype(); }


