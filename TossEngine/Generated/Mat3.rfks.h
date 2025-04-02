#pragma once

#include "../Mat3.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_9783920763335911380u = Mat3::staticGetArchetype(); }

rfk::Class const& Mat3::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Mat3", 9783920763335911380u, sizeof(Mat3), 1);
if (!initialized) {
initialized = true;
Mat3::_rfk_registerChildClass<Mat3>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Mat3>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Mat3>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Mat3>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Mat3>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Mat3>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Mat3>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Mat3>() noexcept { return &Mat3::staticGetArchetype(); }


