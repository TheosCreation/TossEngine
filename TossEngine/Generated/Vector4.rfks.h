#pragma once

#include "../Vector4.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_11237987390232896524u = Vector4::staticGetArchetype(); }

rfk::Class const& Vector4::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Vector4", 11237987390232896524u, sizeof(Vector4), 1);
if (!initialized) {
initialized = true;
Vector4::_rfk_registerChildClass<Vector4>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Vector4>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Vector4>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Vector4>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Vector4>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Vector4>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Vector4>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Vector4>() noexcept { return &Vector4::staticGetArchetype(); }


