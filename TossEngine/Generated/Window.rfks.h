#pragma once

#include "../Window.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_12067293938491365867u = Window::staticGetArchetype(); }

rfk::Class const& Window::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Window", 12067293938491365867u, sizeof(Window), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Resizable>(), static_cast<rfk::EAccessSpecifier>(1));
Window::_rfk_registerChildClass<Window>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Window>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Window>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Window>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Window>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Window>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Window>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Window>() noexcept { return &Window::staticGetArchetype(); }


