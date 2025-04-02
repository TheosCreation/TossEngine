#pragma once

#include "../Renderer.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_347318129641433900u = Renderer::staticGetArchetype(); }

rfk::Class const& Renderer::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Renderer", 347318129641433900u, sizeof(Renderer), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Component>(), static_cast<rfk::EAccessSpecifier>(1));
Renderer::_rfk_registerChildClass<Renderer>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Renderer>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Renderer>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Renderer>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Renderer>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Renderer>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Renderer>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Renderer>() noexcept { return &Renderer::staticGetArchetype(); }


