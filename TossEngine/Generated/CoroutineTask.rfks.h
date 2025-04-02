#pragma once

#include "../CoroutineTask.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_3768004370113228634u = CoroutineTask::staticGetArchetype(); }

rfk::Class const& CoroutineTask::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("CoroutineTask", 3768004370113228634u, sizeof(CoroutineTask), 1);
if (!initialized) {
initialized = true;
CoroutineTask::_rfk_registerChildClass<CoroutineTask>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<CoroutineTask>>(),new rfk::NonMemberFunction<rfk::SharedPtr<CoroutineTask>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<CoroutineTask>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<CoroutineTask>>(),new rfk::NonMemberFunction<rfk::UniquePtr<CoroutineTask>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<CoroutineTask>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
type.setNestedArchetypesCapacity(1);
type.addNestedArchetype(rfk::getArchetype<CoroutineTask::promise_type>(), static_cast<rfk::EAccessSpecifier>(1));
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<CoroutineTask>() noexcept { return &CoroutineTask::staticGetArchetype(); }

rfk::Struct const& CoroutineTask::promise_type::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("promise_type", 16749007553080524831u, sizeof(promise_type), 0);
if (!initialized) {
initialized = true;
promise_type::_rfk_registerChildClass<promise_type>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<promise_type>>(),new rfk::NonMemberFunction<rfk::SharedPtr<promise_type>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<promise_type>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<promise_type>>(),new rfk::NonMemberFunction<rfk::UniquePtr<promise_type>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<promise_type>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<CoroutineTask::promise_type>() noexcept { return &CoroutineTask::promise_type::staticGetArchetype(); }


