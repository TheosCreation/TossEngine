#pragma once

#include "../Vector2.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_11237985191209640102u = Vector2::staticGetArchetype(); }

rfk::Class const& Vector2::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Vector2", 11237985191209640102u, sizeof(Vector2), 1);
if (!initialized) {
initialized = true;
Vector2::_rfk_registerChildClass<Vector2>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Vector2>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Vector2>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Vector2>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Vector2>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Vector2>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Vector2>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Vector2>() noexcept { return &Vector2::staticGetArchetype(); }


