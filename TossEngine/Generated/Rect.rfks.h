#pragma once

#include "../Rect.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_2521235295261752997u = Rect::staticGetArchetype(); }

rfk::Class const& Rect::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Rect", 2521235295261752997u, sizeof(Rect), 1);
if (!initialized) {
initialized = true;
Rect::_rfk_registerChildClass<Rect>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Rect>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Rect>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Rect>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Rect>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Rect>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Rect>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Rect>() noexcept { return &Rect::staticGetArchetype(); }


