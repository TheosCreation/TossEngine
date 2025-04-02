#pragma once

#include "../Image.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_12704835854884233800u = Image::staticGetArchetype(); }

rfk::Class const& Image::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Image", 12704835854884233800u, sizeof(Image), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Renderer>(), static_cast<rfk::EAccessSpecifier>(1));
Image::_rfk_registerChildClass<Image>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Image>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Image>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Image>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Image>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Image>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Image>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Image>() noexcept { return &Image::staticGetArchetype(); }


