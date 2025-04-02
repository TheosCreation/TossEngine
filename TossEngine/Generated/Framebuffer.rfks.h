#pragma once

#include "../Framebuffer.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_9284184048858557746u = Framebuffer::staticGetArchetype(); }

rfk::Class const& Framebuffer::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Framebuffer", 9284184048858557746u, sizeof(Framebuffer), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Resizable>(), static_cast<rfk::EAccessSpecifier>(1));
Framebuffer::_rfk_registerChildClass<Framebuffer>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Framebuffer>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Framebuffer>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Framebuffer>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Framebuffer>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Framebuffer>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Framebuffer>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Framebuffer>() noexcept { return &Framebuffer::staticGetArchetype(); }


