#pragma once

#include "../Sound.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_11715099033136510770u = Sound::staticGetArchetype(); }

rfk::Class const& Sound::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Sound", 11715099033136510770u, sizeof(Sound), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Resource>(), static_cast<rfk::EAccessSpecifier>(1));
Sound::_rfk_registerChildClass<Sound>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Sound>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Sound>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Sound>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Sound>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Sound>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Sound>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Sound>() noexcept { return &Sound::staticGetArchetype(); }


