#pragma once

#include "../JsonUtility.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_3824840137144555219u = JsonUtility::staticGetArchetype(); }

rfk::Class const& JsonUtility::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("JsonUtility", 3824840137144555219u, sizeof(JsonUtility), 1);
if (!initialized) {
initialized = true;
JsonUtility::_rfk_registerChildClass<JsonUtility>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<JsonUtility>>(),new rfk::NonMemberFunction<rfk::SharedPtr<JsonUtility>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<JsonUtility>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<JsonUtility>>(),new rfk::NonMemberFunction<rfk::UniquePtr<JsonUtility>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<JsonUtility>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<JsonUtility>() noexcept { return &JsonUtility::staticGetArchetype(); }


