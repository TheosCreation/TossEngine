#pragma once

#include "../Quaternion.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_8337722352575697635u = Quaternion::staticGetArchetype(); }

rfk::Class const& Quaternion::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Quaternion", 8337722352575697635u, sizeof(Quaternion), 1);
if (!initialized) {
initialized = true;
Quaternion::_rfk_registerChildClass<Quaternion>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Quaternion>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Quaternion>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Quaternion>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Quaternion>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Quaternion>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Quaternion>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Quaternion>() noexcept { return &Quaternion::staticGetArchetype(); }


