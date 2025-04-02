#pragma once

#include "../MeshRenderer.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_213354168814039885u = MeshRenderer::staticGetArchetype(); }

rfk::Class const& MeshRenderer::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("MeshRenderer", 213354168814039885u, sizeof(MeshRenderer), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Renderer>(), static_cast<rfk::EAccessSpecifier>(1));
MeshRenderer::_rfk_registerChildClass<MeshRenderer>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<MeshRenderer>>(),new rfk::NonMemberFunction<rfk::SharedPtr<MeshRenderer>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<MeshRenderer>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<MeshRenderer>>(),new rfk::NonMemberFunction<rfk::UniquePtr<MeshRenderer>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<MeshRenderer>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<MeshRenderer>() noexcept { return &MeshRenderer::staticGetArchetype(); }


