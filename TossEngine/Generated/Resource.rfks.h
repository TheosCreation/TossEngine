#pragma once

#include "../Resource.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_10439140688310071455u = Resource::staticGetArchetype(); }

rfk::Class const& Resource::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Resource", 10439140688310071455u, sizeof(Resource), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<ISelectable>(), static_cast<rfk::EAccessSpecifier>(1));
Resource::_rfk_registerChildClass<Resource>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Resource>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Resource>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Resource>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Resource>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Resource>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Resource>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Resource>() noexcept { return &Resource::staticGetArchetype(); }


