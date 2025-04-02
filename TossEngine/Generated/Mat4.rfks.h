#pragma once

#include "../Mat4.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_9783919663824283169u = Mat4::staticGetArchetype(); }

rfk::Class const& Mat4::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Mat4", 9783919663824283169u, sizeof(Mat4), 1);
if (!initialized) {
initialized = true;
Mat4::_rfk_registerChildClass<Mat4>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Mat4>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Mat4>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Mat4>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Mat4>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Mat4>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Mat4>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Mat4>() noexcept { return &Mat4::staticGetArchetype(); }


