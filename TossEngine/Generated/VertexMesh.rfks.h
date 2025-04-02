#pragma once

#include "../VertexMesh.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_8462618205158972060u = VertexMesh::staticGetArchetype(); }

rfk::Class const& VertexMesh::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("VertexMesh", 8462618205158972060u, sizeof(VertexMesh), 1);
if (!initialized) {
initialized = true;
VertexMesh::_rfk_registerChildClass<VertexMesh>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<VertexMesh>>(),new rfk::NonMemberFunction<rfk::SharedPtr<VertexMesh>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<VertexMesh>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<VertexMesh>>(),new rfk::NonMemberFunction<rfk::UniquePtr<VertexMesh>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<VertexMesh>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<VertexMesh>() noexcept { return &VertexMesh::staticGetArchetype(); }


