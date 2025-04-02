#pragma once

#include "../GraphicsEngine.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_18015641046568531876u = GraphicsEngine::staticGetArchetype(); }

rfk::Class const& GraphicsEngine::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("GraphicsEngine", 18015641046568531876u, sizeof(GraphicsEngine), 1);
if (!initialized) {
initialized = true;
GraphicsEngine::_rfk_registerChildClass<GraphicsEngine>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<GraphicsEngine>>(),new rfk::NonMemberFunction<rfk::SharedPtr<GraphicsEngine>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<GraphicsEngine>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<GraphicsEngine>>(),new rfk::NonMemberFunction<rfk::UniquePtr<GraphicsEngine>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<GraphicsEngine>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<GraphicsEngine>() noexcept { return &GraphicsEngine::staticGetArchetype(); }


