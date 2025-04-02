#pragma once

#include "../VertexArrayObject.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_2261461623013015781u = VertexArrayObject::staticGetArchetype(); }

rfk::Class const& VertexArrayObject::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("VertexArrayObject", 2261461623013015781u, sizeof(VertexArrayObject), 1);
if (!initialized) {
initialized = true;
VertexArrayObject::_rfk_registerChildClass<VertexArrayObject>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<VertexArrayObject>>(),new rfk::NonMemberFunction<rfk::SharedPtr<VertexArrayObject>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<VertexArrayObject>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<VertexArrayObject>>(),new rfk::NonMemberFunction<rfk::UniquePtr<VertexArrayObject>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<VertexArrayObject>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<VertexArrayObject>() noexcept { return &VertexArrayObject::staticGetArchetype(); }


