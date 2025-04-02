#pragma once

#include "../NewSerializable.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_10701075559587321965u = SerializableBase::staticGetArchetype(); }

rfk::Class const& SerializableBase::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("SerializableBase", 10701075559587321965u, sizeof(SerializableBase), 1);
if (!initialized) {
initialized = true;
SerializableBase::_rfk_registerChildClass<SerializableBase>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<SerializableBase>>(),new rfk::NonMemberFunction<rfk::SharedPtr<SerializableBase>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<SerializableBase>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<SerializableBase>>(),new rfk::NonMemberFunction<rfk::UniquePtr<SerializableBase>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<SerializableBase>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<SerializableBase>() noexcept { return &SerializableBase::staticGetArchetype(); }

template <> TOSSENGINE_API rfk::Archetype const* rfk::getArchetype<NewSerializable>() noexcept {
static bool initialized = false;
static rfk::ClassTemplate type("NewSerializable", 17355248601527932472u, 1);
if (!initialized) {
initialized = true;
{ 
static rfk::TypeTemplateParameter templateParameter("Derived");
type.addTemplateParameter(templateParameter);
}
}return &type; }

namespace rfk::generated { static rfk::ArchetypeRegisterer const register_17355248601527932472u = *rfk::getArchetype<::NewSerializable>(); }


