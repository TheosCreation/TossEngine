#pragma once

#include "../Texture2D.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_3185127993621166460u = Texture2D::staticGetArchetype(); }

rfk::Class const& Texture2D::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Texture2D", 3185127993621166460u, sizeof(Texture2D), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Texture>(), static_cast<rfk::EAccessSpecifier>(1));
Texture2D::_rfk_registerChildClass<Texture2D>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Texture2D>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Texture2D>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Texture2D>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Texture2D>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Texture2D>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Texture2D>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Texture2D>() noexcept { return &Texture2D::staticGetArchetype(); }


