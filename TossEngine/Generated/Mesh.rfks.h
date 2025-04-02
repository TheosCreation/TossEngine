#pragma once

#include "../Mesh.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_12155177018125308950u = Mesh::staticGetArchetype(); }

rfk::Class const& Mesh::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Mesh", 12155177018125308950u, sizeof(Mesh), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Resource>(), static_cast<rfk::EAccessSpecifier>(1));
Mesh::_rfk_registerChildClass<Mesh>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Mesh>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Mesh>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Mesh>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Mesh>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Mesh>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Mesh>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Mesh>() noexcept { return &Mesh::staticGetArchetype(); }


