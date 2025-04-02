#pragma once

#include "../TestClass2.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_10287496089702573345u = TestClass2::staticGetArchetype(); }

rfk::Class const& TestClass2::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("TestClass2", 10287496089702573345u, sizeof(TestClass2), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<TestClass>(), static_cast<rfk::EAccessSpecifier>(1));
TestClass2::_rfk_registerChildClass<TestClass2>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<TestClass2>>(),new rfk::NonMemberFunction<rfk::SharedPtr<TestClass2>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<TestClass2>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<TestClass2>>(),new rfk::NonMemberFunction<rfk::UniquePtr<TestClass2>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<TestClass2>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<TestClass2>() noexcept { return &TestClass2::staticGetArchetype(); }


