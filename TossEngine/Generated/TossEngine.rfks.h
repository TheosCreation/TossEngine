#pragma once

#include "../TossEngine.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_12546767852364273894u = TossEngine::staticGetArchetype(); }

rfk::Class const& TossEngine::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("TossEngine", 12546767852364273894u, sizeof(TossEngine), 1);
if (!initialized) {
initialized = true;
TossEngine::_rfk_registerChildClass<TossEngine>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<TossEngine>>(),new rfk::NonMemberFunction<rfk::SharedPtr<TossEngine>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<TossEngine>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<TossEngine>>(),new rfk::NonMemberFunction<rfk::UniquePtr<TossEngine>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<TossEngine>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<TossEngine>() noexcept { return &TossEngine::staticGetArchetype(); }


