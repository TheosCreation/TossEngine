#pragma once

#include "../Scene.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_14627978680582598289u = Scene::staticGetArchetype(); }

rfk::Class const& Scene::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Scene", 14627978680582598289u, sizeof(Scene), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Resizable>(), static_cast<rfk::EAccessSpecifier>(1));
Scene::_rfk_registerChildClass<Scene>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Scene>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Scene>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Scene>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Scene>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Scene>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Scene>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Scene>() noexcept { return &Scene::staticGetArchetype(); }


