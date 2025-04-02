#pragma once

#include "../Shader.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_9188089338102985566u = Shader::staticGetArchetype(); }

rfk::Class const& Shader::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Shader", 9188089338102985566u, sizeof(Shader), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Resource>(), static_cast<rfk::EAccessSpecifier>(1));
Shader::_rfk_registerChildClass<Shader>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Shader>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Shader>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Shader>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Shader>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Shader>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Shader>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Shader>() noexcept { return &Shader::staticGetArchetype(); }


