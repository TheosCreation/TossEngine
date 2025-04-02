#pragma once

#include "../ProjectSettings.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_15266865373177807837u = ProjectSettings::staticGetArchetype(); }

rfk::Struct const& ProjectSettings::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("ProjectSettings", 15266865373177807837u, sizeof(ProjectSettings), 0);
if (!initialized) {
initialized = true;
ProjectSettings::_rfk_registerChildClass<ProjectSettings>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<ProjectSettings>>(),new rfk::NonMemberFunction<rfk::SharedPtr<ProjectSettings>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<ProjectSettings>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<ProjectSettings>>(),new rfk::NonMemberFunction<rfk::UniquePtr<ProjectSettings>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<ProjectSettings>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<ProjectSettings>() noexcept { return &ProjectSettings::staticGetArchetype(); }


