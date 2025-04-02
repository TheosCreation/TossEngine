#pragma once

#include "../GeometryBuffer.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_6676646866646191025u = GeometryBuffer::staticGetArchetype(); }

rfk::Class const& GeometryBuffer::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("GeometryBuffer", 6676646866646191025u, sizeof(GeometryBuffer), 1);
if (!initialized) {
initialized = true;
GeometryBuffer::_rfk_registerChildClass<GeometryBuffer>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<GeometryBuffer>>(),new rfk::NonMemberFunction<rfk::SharedPtr<GeometryBuffer>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<GeometryBuffer>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<GeometryBuffer>>(),new rfk::NonMemberFunction<rfk::UniquePtr<GeometryBuffer>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<GeometryBuffer>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<GeometryBuffer>() noexcept { return &GeometryBuffer::staticGetArchetype(); }


