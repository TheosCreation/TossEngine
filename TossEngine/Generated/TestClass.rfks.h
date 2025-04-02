#pragma once

#include "../TestClass.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_13034592833979223017u = TestClass::staticGetArchetype(); }

rfk::Class const& TestClass::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("TestClass", 13034592833979223017u, sizeof(TestClass), 1);
if (!initialized) {
initialized = true;
TestClass::_rfk_registerChildClass<TestClass>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<TestClass>>(),new rfk::NonMemberFunction<rfk::SharedPtr<TestClass>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<TestClass>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<TestClass>>(),new rfk::NonMemberFunction<rfk::UniquePtr<TestClass>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<TestClass>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<TestClass>() noexcept { return &TestClass::staticGetArchetype(); }


