#pragma once

#include "../ParticleSystem.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_11262162319982559268u = ParticleSystem::staticGetArchetype(); }

rfk::Class const& ParticleSystem::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("ParticleSystem", 11262162319982559268u, sizeof(ParticleSystem), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Renderer>(), static_cast<rfk::EAccessSpecifier>(1));
ParticleSystem::_rfk_registerChildClass<ParticleSystem>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<ParticleSystem>>(),new rfk::NonMemberFunction<rfk::SharedPtr<ParticleSystem>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<ParticleSystem>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<ParticleSystem>>(),new rfk::NonMemberFunction<rfk::UniquePtr<ParticleSystem>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<ParticleSystem>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<ParticleSystem>() noexcept { return &ParticleSystem::staticGetArchetype(); }


