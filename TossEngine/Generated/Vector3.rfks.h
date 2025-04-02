#pragma once

#include "../Vector3.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_11237986290721268313u = Vector3::staticGetArchetype(); }

rfk::Class const& Vector3::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Vector3", 11237986290721268313u, sizeof(Vector3), 1);
if (!initialized) {
initialized = true;
Vector3::_rfk_registerChildClass<Vector3>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Vector3>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Vector3>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Vector3>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Vector3>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Vector3>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Vector3>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Vector3>() noexcept { return &Vector3::staticGetArchetype(); }


