#pragma once

#include "../ComponentRegistry.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_5691639982574862381u = ComponentRegistry::staticGetArchetype(); }

rfk::Class const& ComponentRegistry::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("ComponentRegistry", 5691639982574862381u, sizeof(ComponentRegistry), 1);
if (!initialized) {
initialized = true;
ComponentRegistry::_rfk_registerChildClass<ComponentRegistry>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<ComponentRegistry>>(),new rfk::NonMemberFunction<rfk::SharedPtr<ComponentRegistry>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<ComponentRegistry>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<ComponentRegistry>>(),new rfk::NonMemberFunction<rfk::UniquePtr<ComponentRegistry>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<ComponentRegistry>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<ComponentRegistry>() noexcept { return &ComponentRegistry::staticGetArchetype(); }


