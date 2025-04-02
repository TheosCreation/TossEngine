#pragma once

#include "../ScriptLoader.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_9568621867530182447u = ScriptLoader::staticGetArchetype(); }

rfk::Class const& ScriptLoader::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("ScriptLoader", 9568621867530182447u, sizeof(ScriptLoader), 1);
if (!initialized) {
initialized = true;
ScriptLoader::_rfk_registerChildClass<ScriptLoader>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<ScriptLoader>>(),new rfk::NonMemberFunction<rfk::SharedPtr<ScriptLoader>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<ScriptLoader>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<ScriptLoader>>(),new rfk::NonMemberFunction<rfk::UniquePtr<ScriptLoader>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<ScriptLoader>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<ScriptLoader>() noexcept { return &ScriptLoader::staticGetArchetype(); }


