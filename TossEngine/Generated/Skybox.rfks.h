#pragma once

#include "../Skybox.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_11773040552951948017u = Skybox::staticGetArchetype(); }

rfk::Class const& Skybox::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Skybox", 11773040552951948017u, sizeof(Skybox), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Renderer>(), static_cast<rfk::EAccessSpecifier>(1));
Skybox::_rfk_registerChildClass<Skybox>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Skybox>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Skybox>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Skybox>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Skybox>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Skybox>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Skybox>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Skybox>() noexcept { return &Skybox::staticGetArchetype(); }


