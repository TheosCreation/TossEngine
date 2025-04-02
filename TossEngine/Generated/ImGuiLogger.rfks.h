#pragma once

#include "../ImGuiLogger.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_10245806638524999736u = ImGuiLogger::staticGetArchetype(); }

rfk::Class const& ImGuiLogger::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("ImGuiLogger", 10245806638524999736u, sizeof(ImGuiLogger), 1);
if (!initialized) {
initialized = true;
ImGuiLogger::_rfk_registerChildClass<ImGuiLogger>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<ImGuiLogger>>(),new rfk::NonMemberFunction<rfk::SharedPtr<ImGuiLogger>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<ImGuiLogger>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<ImGuiLogger>>(),new rfk::NonMemberFunction<rfk::UniquePtr<ImGuiLogger>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<ImGuiLogger>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<ImGuiLogger>() noexcept { return &ImGuiLogger::staticGetArchetype(); }


