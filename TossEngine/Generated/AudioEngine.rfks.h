#pragma once

#include "../AudioEngine.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_18250915763581299809u = AudioEngine::staticGetArchetype(); }

rfk::Class const& AudioEngine::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("AudioEngine", 18250915763581299809u, sizeof(AudioEngine), 1);
if (!initialized) {
initialized = true;
AudioEngine::_rfk_registerChildClass<AudioEngine>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<AudioEngine>>(),new rfk::NonMemberFunction<rfk::SharedPtr<AudioEngine>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<AudioEngine>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<AudioEngine>>(),new rfk::NonMemberFunction<rfk::UniquePtr<AudioEngine>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<AudioEngine>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<AudioEngine>() noexcept { return &AudioEngine::staticGetArchetype(); }

template <> rfk::Function const* rfk::getFunction<static_cast<void(*)(FMOD_RESULT, const char *, int)>(&ERRCHECK_fn)>() noexcept {
static bool initialized = false;
static rfk::Function function("ERRCHECK_fn", 2741669238571437628u, rfk::getType<void>(), new rfk::NonMemberFunction<void (FMOD_RESULT, const char *, int)>(&ERRCHECK_fn), static_cast<rfk::EFunctionFlags>(0));
if (!initialized) {
initialized = true;
function.setParametersCapacity(3);
function.addParameter("result", 0u, rfk::getType<FMOD_RESULT>());
function.addParameter("file", 0u, rfk::getType<const char *>());
function.addParameter("line", 0u, rfk::getType<int>());
;
}return &function; }
namespace rfk::generated { static rfk::DefaultEntityRegisterer const registerer2741669238571437628u = *rfk::getFunction<static_cast<void(*)(FMOD_RESULT, const char *, int)>(&ERRCHECK_fn)>(); }

