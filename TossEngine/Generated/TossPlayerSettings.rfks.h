#pragma once

#include "../TossPlayerSettings.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_9551593081664491276u = TossPlayerSettings::staticGetArchetype(); }

rfk::Struct const& TossPlayerSettings::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("TossPlayerSettings", 9551593081664491276u, sizeof(TossPlayerSettings), 0);
if (!initialized) {
initialized = true;
TossPlayerSettings::_rfk_registerChildClass<TossPlayerSettings>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<TossPlayerSettings>>(),new rfk::NonMemberFunction<rfk::SharedPtr<TossPlayerSettings>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<TossPlayerSettings>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<TossPlayerSettings>>(),new rfk::NonMemberFunction<rfk::UniquePtr<TossPlayerSettings>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<TossPlayerSettings>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<TossPlayerSettings>() noexcept { return &TossPlayerSettings::staticGetArchetype(); }


