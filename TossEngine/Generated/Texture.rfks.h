#pragma once

#include "../Texture.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_15806940960501105090u = Texture::staticGetArchetype(); }

rfk::Class const& Texture::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Texture", 15806940960501105090u, sizeof(Texture), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Resource>(), static_cast<rfk::EAccessSpecifier>(1));
Texture::_rfk_registerChildClass<Texture>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Texture>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Texture>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Texture>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Texture>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Texture>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Texture>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Texture>() noexcept { return &Texture::staticGetArchetype(); }


