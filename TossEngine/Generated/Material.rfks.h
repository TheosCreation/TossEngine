#pragma once

#include "../Material.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_1317015671129832901u = Texture2DBinding::staticGetArchetype(); }

rfk::Struct const& Texture2DBinding::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("Texture2DBinding", 1317015671129832901u, sizeof(Texture2DBinding), 0);
if (!initialized) {
initialized = true;
Texture2DBinding::_rfk_registerChildClass<Texture2DBinding>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Texture2DBinding>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Texture2DBinding>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Texture2DBinding>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Texture2DBinding>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Texture2DBinding>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Texture2DBinding>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Texture2DBinding>() noexcept { return &Texture2DBinding::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_4490184633201042400u = TextureCubeMapBinding::staticGetArchetype(); }

rfk::Struct const& TextureCubeMapBinding::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("TextureCubeMapBinding", 4490184633201042400u, sizeof(TextureCubeMapBinding), 0);
if (!initialized) {
initialized = true;
TextureCubeMapBinding::_rfk_registerChildClass<TextureCubeMapBinding>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<TextureCubeMapBinding>>(),new rfk::NonMemberFunction<rfk::SharedPtr<TextureCubeMapBinding>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<TextureCubeMapBinding>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<TextureCubeMapBinding>>(),new rfk::NonMemberFunction<rfk::UniquePtr<TextureCubeMapBinding>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<TextureCubeMapBinding>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<TextureCubeMapBinding>() noexcept { return &TextureCubeMapBinding::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_10546122322523466346u = Material::staticGetArchetype(); }

rfk::Class const& Material::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("Material", 10546122322523466346u, sizeof(Material), 1);
if (!initialized) {
initialized = true;
type.setDirectParentsCapacity(1);
type.addDirectParent(rfk::getArchetype<Resource>(), static_cast<rfk::EAccessSpecifier>(1));
Material::_rfk_registerChildClass<Material>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Material>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Material>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Material>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Material>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Material>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Material>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Material>() noexcept { return &Material::staticGetArchetype(); }


