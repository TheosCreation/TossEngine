#pragma once

#include "../Physics.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_4692049323270959543u = RaycastHit::staticGetArchetype(); }

rfk::Struct const& RaycastHit::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("RaycastHit", 4692049323270959543u, sizeof(RaycastHit), 0);
if (!initialized) {
initialized = true;
RaycastHit::_rfk_registerChildClass<RaycastHit>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<RaycastHit>>(),new rfk::NonMemberFunction<rfk::SharedPtr<RaycastHit>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<RaycastHit>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<RaycastHit>>(),new rfk::NonMemberFunction<rfk::UniquePtr<RaycastHit>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<RaycastHit>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<RaycastHit>() noexcept { return &RaycastHit::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_3600558425080952601u = RaycastCallback::staticGetArchetype(); }

rfk::Struct const& RaycastCallback::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("RaycastCallback", 3600558425080952601u, sizeof(RaycastCallback), 0);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<reactphysics3d::RaycastCallback>(), static_cast<rfk::EAccessSpecifier>(1));
RaycastCallback::_rfk_registerChildClass<RaycastCallback>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<RaycastCallback>>(),new rfk::NonMemberFunction<rfk::SharedPtr<RaycastCallback>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<RaycastCallback>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<RaycastCallback>>(),new rfk::NonMemberFunction<rfk::UniquePtr<RaycastCallback>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<RaycastCallback>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<RaycastCallback>() noexcept { return &RaycastCallback::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_10533060058756583944u = Physics::staticGetArchetype(); }

rfk::Class const& Physics::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Physics", 10533060058756583944u, sizeof(Physics), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<rp3d::CollisionCallback>(), static_cast<rfk::EAccessSpecifier>(1));
Physics::_rfk_registerChildClass<Physics>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Physics>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Physics>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Physics>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Physics>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Physics>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Physics>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Physics>() noexcept { return &Physics::staticGetArchetype(); }


