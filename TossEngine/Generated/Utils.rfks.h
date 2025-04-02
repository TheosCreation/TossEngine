#pragma once

#include "../Utils.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { 
 static rfk::NamespaceFragment const& getNamespaceFragment_9313269372163894171u_16872654211826741289() noexcept {
static rfk::NamespaceFragment fragment("QuaternionUtils", 9313269372163894171u);
static bool initialized = false;
if (!initialized) {
initialized = true;
fragment.setNestedEntitiesCapacity(1u);
fragment.addNestedEntity(*rfk::getFunction<static_cast<Quaternion(*)(const glm::vec3 &, const glm::vec3 &)>(&QuaternionUtils::LookAt)>());
}
return fragment; }
static rfk::NamespaceFragmentRegisterer const namespaceFragmentRegisterer_9313269372163894171u_16872654211826741289(rfk::generated::getNamespaceFragment_9313269372163894171u_16872654211826741289());
 }
template <> rfk::Function const* rfk::getFunction<static_cast<Quaternion(*)(const glm::vec3 &, const glm::vec3 &)>(&QuaternionUtils::LookAt)>() noexcept {
static bool initialized = false;
static rfk::Function function("LookAt", 731153581942724672u, rfk::getType<Quaternion>(), new rfk::NonMemberFunction<Quaternion (const glm::vec3 &, const glm::vec3 &)>(&QuaternionUtils::LookAt), static_cast<rfk::EFunctionFlags>(2));
if (!initialized) {
initialized = true;
function.setParametersCapacity(2);
function.addParameter("direction", 0u, rfk::getType<const glm::vec3 &>());
function.addParameter("up", 0u, rfk::getType<const glm::vec3 &>());
;
}return &function; }
namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_2620958058495390501u = Vertex::staticGetArchetype(); }

rfk::Struct const& Vertex::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("Vertex", 2620958058495390501u, sizeof(Vertex), 0);
if (!initialized) {
initialized = true;
Vertex::_rfk_registerChildClass<Vertex>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Vertex>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Vertex>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Vertex>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Vertex>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Vertex>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Vertex>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Vertex>() noexcept { return &Vertex::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_13339590791364480360u = DebugVertex::staticGetArchetype(); }

rfk::Struct const& DebugVertex::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("DebugVertex", 13339590791364480360u, sizeof(DebugVertex), 0);
if (!initialized) {
initialized = true;
DebugVertex::_rfk_registerChildClass<DebugVertex>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<DebugVertex>>(),new rfk::NonMemberFunction<rfk::SharedPtr<DebugVertex>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<DebugVertex>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<DebugVertex>>(),new rfk::NonMemberFunction<rfk::UniquePtr<DebugVertex>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<DebugVertex>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<DebugVertex>() noexcept { return &DebugVertex::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_620406438925976655u = VertexAttribute::staticGetArchetype(); }

rfk::Struct const& VertexAttribute::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("VertexAttribute", 620406438925976655u, sizeof(VertexAttribute), 0);
if (!initialized) {
initialized = true;
VertexAttribute::_rfk_registerChildClass<VertexAttribute>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<VertexAttribute>>(),new rfk::NonMemberFunction<rfk::SharedPtr<VertexAttribute>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<VertexAttribute>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<VertexAttribute>>(),new rfk::NonMemberFunction<rfk::UniquePtr<VertexAttribute>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<VertexAttribute>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<VertexAttribute>() noexcept { return &VertexAttribute::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_13297885131338198262u = VertexBufferDesc::staticGetArchetype(); }

rfk::Struct const& VertexBufferDesc::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("VertexBufferDesc", 13297885131338198262u, sizeof(VertexBufferDesc), 0);
if (!initialized) {
initialized = true;
VertexBufferDesc::_rfk_registerChildClass<VertexBufferDesc>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<VertexBufferDesc>>(),new rfk::NonMemberFunction<rfk::SharedPtr<VertexBufferDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<VertexBufferDesc>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<VertexBufferDesc>>(),new rfk::NonMemberFunction<rfk::UniquePtr<VertexBufferDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<VertexBufferDesc>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<VertexBufferDesc>() noexcept { return &VertexBufferDesc::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_3238851499365635850u = IndexBufferDesc::staticGetArchetype(); }

rfk::Struct const& IndexBufferDesc::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("IndexBufferDesc", 3238851499365635850u, sizeof(IndexBufferDesc), 0);
if (!initialized) {
initialized = true;
IndexBufferDesc::_rfk_registerChildClass<IndexBufferDesc>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<IndexBufferDesc>>(),new rfk::NonMemberFunction<rfk::SharedPtr<IndexBufferDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<IndexBufferDesc>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<IndexBufferDesc>>(),new rfk::NonMemberFunction<rfk::UniquePtr<IndexBufferDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<IndexBufferDesc>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<IndexBufferDesc>() noexcept { return &IndexBufferDesc::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_11168547885788905343u = ShaderDesc::staticGetArchetype(); }

rfk::Struct const& ShaderDesc::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("ShaderDesc", 11168547885788905343u, sizeof(ShaderDesc), 0);
if (!initialized) {
initialized = true;
ShaderDesc::_rfk_registerChildClass<ShaderDesc>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<ShaderDesc>>(),new rfk::NonMemberFunction<rfk::SharedPtr<ShaderDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<ShaderDesc>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<ShaderDesc>>(),new rfk::NonMemberFunction<rfk::UniquePtr<ShaderDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<ShaderDesc>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<ShaderDesc>() noexcept { return &ShaderDesc::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_2221437513243590423u = MeshDesc::staticGetArchetype(); }

rfk::Struct const& MeshDesc::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("MeshDesc", 2221437513243590423u, sizeof(MeshDesc), 0);
if (!initialized) {
initialized = true;
MeshDesc::_rfk_registerChildClass<MeshDesc>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<MeshDesc>>(),new rfk::NonMemberFunction<rfk::SharedPtr<MeshDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<MeshDesc>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<MeshDesc>>(),new rfk::NonMemberFunction<rfk::UniquePtr<MeshDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<MeshDesc>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<MeshDesc>() noexcept { return &MeshDesc::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_6032087050921078814u = UniformBufferDesc::staticGetArchetype(); }

rfk::Struct const& UniformBufferDesc::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("UniformBufferDesc", 6032087050921078814u, sizeof(UniformBufferDesc), 0);
if (!initialized) {
initialized = true;
UniformBufferDesc::_rfk_registerChildClass<UniformBufferDesc>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<UniformBufferDesc>>(),new rfk::NonMemberFunction<rfk::SharedPtr<UniformBufferDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<UniformBufferDesc>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<UniformBufferDesc>>(),new rfk::NonMemberFunction<rfk::UniquePtr<UniformBufferDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<UniformBufferDesc>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<UniformBufferDesc>() noexcept { return &UniformBufferDesc::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_8738565980990763533u = UniformData::staticGetArchetype(); }

rfk::Struct const& UniformData::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("UniformData", 8738565980990763533u, sizeof(UniformData), 0);
if (!initialized) {
initialized = true;
UniformData::_rfk_registerChildClass<UniformData>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<UniformData>>(),new rfk::NonMemberFunction<rfk::SharedPtr<UniformData>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<UniformData>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<UniformData>>(),new rfk::NonMemberFunction<rfk::UniquePtr<UniformData>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<UniformData>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<UniformData>() noexcept { return &UniformData::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_11381140263443852414u = NewExtraTextureData::staticGetArchetype(); }

rfk::Struct const& NewExtraTextureData::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("NewExtraTextureData", 11381140263443852414u, sizeof(NewExtraTextureData), 0);
if (!initialized) {
initialized = true;
NewExtraTextureData::_rfk_registerChildClass<NewExtraTextureData>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<NewExtraTextureData>>(),new rfk::NonMemberFunction<rfk::SharedPtr<NewExtraTextureData>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<NewExtraTextureData>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<NewExtraTextureData>>(),new rfk::NonMemberFunction<rfk::UniquePtr<NewExtraTextureData>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<NewExtraTextureData>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<NewExtraTextureData>() noexcept { return &NewExtraTextureData::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_16147917549793238269u = NewUniformData::staticGetArchetype(); }

rfk::Struct const& NewUniformData::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("NewUniformData", 16147917549793238269u, sizeof(NewUniformData), 0);
if (!initialized) {
initialized = true;
NewUniformData::_rfk_registerChildClass<NewUniformData>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<NewUniformData>>(),new rfk::NonMemberFunction<rfk::SharedPtr<NewUniformData>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<NewUniformData>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<NewUniformData>>(),new rfk::NonMemberFunction<rfk::UniquePtr<NewUniformData>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<NewUniformData>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<NewUniformData>() noexcept { return &NewUniformData::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_94252775372968381u = Texture2DDesc::staticGetArchetype(); }

rfk::Struct const& Texture2DDesc::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("Texture2DDesc", 94252775372968381u, sizeof(Texture2DDesc), 0);
if (!initialized) {
initialized = true;
Texture2DDesc::_rfk_registerChildClass<Texture2DDesc>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Texture2DDesc>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Texture2DDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Texture2DDesc>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Texture2DDesc>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Texture2DDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Texture2DDesc>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Texture2DDesc>() noexcept { return &Texture2DDesc::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_16757675733334032986u = HeightMapInfo::staticGetArchetype(); }

rfk::Struct const& HeightMapInfo::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("HeightMapInfo", 16757675733334032986u, sizeof(HeightMapInfo), 0);
if (!initialized) {
initialized = true;
HeightMapInfo::_rfk_registerChildClass<HeightMapInfo>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<HeightMapInfo>>(),new rfk::NonMemberFunction<rfk::SharedPtr<HeightMapInfo>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<HeightMapInfo>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<HeightMapInfo>>(),new rfk::NonMemberFunction<rfk::UniquePtr<HeightMapInfo>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<HeightMapInfo>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<HeightMapInfo>() noexcept { return &HeightMapInfo::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_8645800920699554979u = HeightMapDesc::staticGetArchetype(); }

rfk::Struct const& HeightMapDesc::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("HeightMapDesc", 8645800920699554979u, sizeof(HeightMapDesc), 0);
if (!initialized) {
initialized = true;
HeightMapDesc::_rfk_registerChildClass<HeightMapDesc>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<HeightMapDesc>>(),new rfk::NonMemberFunction<rfk::SharedPtr<HeightMapDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<HeightMapDesc>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<HeightMapDesc>>(),new rfk::NonMemberFunction<rfk::UniquePtr<HeightMapDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<HeightMapDesc>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<HeightMapDesc>() noexcept { return &HeightMapDesc::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_4768188023978926714u = TextureCubeMapDesc::staticGetArchetype(); }

rfk::Struct const& TextureCubeMapDesc::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("TextureCubeMapDesc", 4768188023978926714u, sizeof(TextureCubeMapDesc), 0);
if (!initialized) {
initialized = true;
TextureCubeMapDesc::_rfk_registerChildClass<TextureCubeMapDesc>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<TextureCubeMapDesc>>(),new rfk::NonMemberFunction<rfk::SharedPtr<TextureCubeMapDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<TextureCubeMapDesc>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<TextureCubeMapDesc>>(),new rfk::NonMemberFunction<rfk::UniquePtr<TextureCubeMapDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<TextureCubeMapDesc>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<TextureCubeMapDesc>() noexcept { return &TextureCubeMapDesc::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_52505688226525651u = SoundDesc::staticGetArchetype(); }

rfk::Struct const& SoundDesc::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("SoundDesc", 52505688226525651u, sizeof(SoundDesc), 0);
if (!initialized) {
initialized = true;
SoundDesc::_rfk_registerChildClass<SoundDesc>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<SoundDesc>>(),new rfk::NonMemberFunction<rfk::SharedPtr<SoundDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<SoundDesc>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<SoundDesc>>(),new rfk::NonMemberFunction<rfk::UniquePtr<SoundDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<SoundDesc>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<SoundDesc>() noexcept { return &SoundDesc::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_10256190192513373338u = UniformBinding::staticGetArchetype(); }

rfk::Struct const& UniformBinding::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("UniformBinding", 10256190192513373338u, sizeof(UniformBinding), 0);
if (!initialized) {
initialized = true;
UniformBinding::_rfk_registerChildClass<UniformBinding>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<UniformBinding>>(),new rfk::NonMemberFunction<rfk::SharedPtr<UniformBinding>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<UniformBinding>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<UniformBinding>>(),new rfk::NonMemberFunction<rfk::UniquePtr<UniformBinding>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<UniformBinding>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<UniformBinding>() noexcept { return &UniformBinding::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_16397860657848201707u = MaterialDesc::staticGetArchetype(); }

rfk::Struct const& MaterialDesc::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("MaterialDesc", 16397860657848201707u, sizeof(MaterialDesc), 0);
if (!initialized) {
initialized = true;
MaterialDesc::_rfk_registerChildClass<MaterialDesc>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<MaterialDesc>>(),new rfk::NonMemberFunction<rfk::SharedPtr<MaterialDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<MaterialDesc>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<MaterialDesc>>(),new rfk::NonMemberFunction<rfk::UniquePtr<MaterialDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<MaterialDesc>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<MaterialDesc>() noexcept { return &MaterialDesc::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_7232699130693052412u = PhysicsMaterialDesc::staticGetArchetype(); }

rfk::Struct const& PhysicsMaterialDesc::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("PhysicsMaterialDesc", 7232699130693052412u, sizeof(PhysicsMaterialDesc), 0);
if (!initialized) {
initialized = true;
PhysicsMaterialDesc::_rfk_registerChildClass<PhysicsMaterialDesc>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<PhysicsMaterialDesc>>(),new rfk::NonMemberFunction<rfk::SharedPtr<PhysicsMaterialDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<PhysicsMaterialDesc>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<PhysicsMaterialDesc>>(),new rfk::NonMemberFunction<rfk::UniquePtr<PhysicsMaterialDesc>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<PhysicsMaterialDesc>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<PhysicsMaterialDesc>() noexcept { return &PhysicsMaterialDesc::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_16668306092055315026u = Color::staticGetArchetype(); }

rfk::Struct const& Color::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("Color", 16668306092055315026u, sizeof(Color), 0);
if (!initialized) {
initialized = true;
Color::_rfk_registerChildClass<Color>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<Color>>(),new rfk::NonMemberFunction<rfk::SharedPtr<Color>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<Color>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<Color>>(),new rfk::NonMemberFunction<rfk::UniquePtr<Color>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<Color>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<Color>() noexcept { return &Color::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_7197500502103587331u = DirectionalLightData::staticGetArchetype(); }

rfk::Struct const& DirectionalLightData::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("DirectionalLightData", 7197500502103587331u, sizeof(DirectionalLightData), 0);
if (!initialized) {
initialized = true;
DirectionalLightData::_rfk_registerChildClass<DirectionalLightData>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<DirectionalLightData>>(),new rfk::NonMemberFunction<rfk::SharedPtr<DirectionalLightData>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<DirectionalLightData>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<DirectionalLightData>>(),new rfk::NonMemberFunction<rfk::UniquePtr<DirectionalLightData>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<DirectionalLightData>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<DirectionalLightData>() noexcept { return &DirectionalLightData::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_7393892997781780421u = PointLightData::staticGetArchetype(); }

rfk::Struct const& PointLightData::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("PointLightData", 7393892997781780421u, sizeof(PointLightData), 0);
if (!initialized) {
initialized = true;
PointLightData::_rfk_registerChildClass<PointLightData>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<PointLightData>>(),new rfk::NonMemberFunction<rfk::SharedPtr<PointLightData>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<PointLightData>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<PointLightData>>(),new rfk::NonMemberFunction<rfk::UniquePtr<PointLightData>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<PointLightData>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<PointLightData>() noexcept { return &PointLightData::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_15742992834227714757u = SpotLightData::staticGetArchetype(); }

rfk::Struct const& SpotLightData::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("SpotLightData", 15742992834227714757u, sizeof(SpotLightData), 0);
if (!initialized) {
initialized = true;
SpotLightData::_rfk_registerChildClass<SpotLightData>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<SpotLightData>>(),new rfk::NonMemberFunction<rfk::SharedPtr<SpotLightData>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<SpotLightData>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<SpotLightData>>(),new rfk::NonMemberFunction<rfk::UniquePtr<SpotLightData>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<SpotLightData>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<SpotLightData>() noexcept { return &SpotLightData::staticGetArchetype(); }

template <> rfk::Enum const* rfk::getEnum<CameraType>() noexcept
{
static bool initialized = false;
static rfk::Enum type("CameraType", 17708689933374599070u, rfk::getArchetype<int>());
if (!initialized) {
initialized = true;
rfk::EnumValue* enumValue = nullptr;
type.setEnumValuesCapacity(2);
enumValue = type.addEnumValue("Orthogonal", 569684382183995929u, 0);
enumValue = type.addEnumValue("Perspective", 6267611147222090776u, 1);
}
return &type; }
namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_17708689933374599070u = *rfk::getEnum<CameraType>(); }
template <> rfk::Enum const* rfk::getEnum<RenderingPath>() noexcept
{
static bool initialized = false;
static rfk::Enum type("RenderingPath", 13719653550453925572u, rfk::getArchetype<int>());
if (!initialized) {
initialized = true;
rfk::EnumValue* enumValue = nullptr;
type.setEnumValuesCapacity(3);
enumValue = type.addEnumValue("Deferred", 615314174213130727u, 0);
enumValue = type.addEnumValue("Forward", 2282048513614330321u, 1);
enumValue = type.addEnumValue("Unknown", 9091548303323809102u, 2);
}
return &type; }
namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_13719653550453925572u = *rfk::getEnum<RenderingPath>(); }
template <> rfk::Enum const* rfk::getEnum<TriangleType>() noexcept
{
static bool initialized = false;
static rfk::Enum type("TriangleType", 14268102062889648925u, rfk::getArchetype<int>());
if (!initialized) {
initialized = true;
rfk::EnumValue* enumValue = nullptr;
type.setEnumValuesCapacity(3);
enumValue = type.addEnumValue("TriangleList", 17063336937638671587u, 0);
enumValue = type.addEnumValue("TriangleStrip", 13521933103579319635u, 1);
enumValue = type.addEnumValue("Points", 15211432309277935812u, 2);
}
return &type; }
namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_14268102062889648925u = *rfk::getEnum<TriangleType>(); }
template <> rfk::Enum const* rfk::getEnum<LineType>() noexcept
{
static bool initialized = false;
static rfk::Enum type("LineType", 1865373815320203375u, rfk::getArchetype<int>());
if (!initialized) {
initialized = true;
rfk::EnumValue* enumValue = nullptr;
type.setEnumValuesCapacity(2);
enumValue = type.addEnumValue("Lines", 4715421022505626388u, 0);
enumValue = type.addEnumValue("LineStrip", 8092119090265027271u, 1);
}
return &type; }
namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_1865373815320203375u = *rfk::getEnum<LineType>(); }
template <> rfk::Enum const* rfk::getEnum<CullType>() noexcept
{
static bool initialized = false;
static rfk::Enum type("CullType", 3590183066813042877u, rfk::getArchetype<int>());
if (!initialized) {
initialized = true;
rfk::EnumValue* enumValue = nullptr;
type.setEnumValuesCapacity(4);
enumValue = type.addEnumValue("BackFace", 325980537536162257u, 0);
enumValue = type.addEnumValue("FrontFace", 13453326922345270903u, 1);
enumValue = type.addEnumValue("Both", 6596143271259164108u, 2);
enumValue = type.addEnumValue("None", 5415313012155872569u, 3);
}
return &type; }
namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_3590183066813042877u = *rfk::getEnum<CullType>(); }
template <> rfk::Enum const* rfk::getEnum<BlendType>() noexcept
{
static bool initialized = false;
static rfk::Enum type("BlendType", 10684716730739981600u, rfk::getArchetype<int>());
if (!initialized) {
initialized = true;
rfk::EnumValue* enumValue = nullptr;
type.setEnumValuesCapacity(14);
enumValue = type.addEnumValue("Zero", 5193143554224695842u, 0);
enumValue = type.addEnumValue("One", 17966986972078427108u, 1);
enumValue = type.addEnumValue("SrcColor", 8340966269515663241u, 2);
enumValue = type.addEnumValue("OneMinusSrcColor", 9338629678655472221u, 3);
enumValue = type.addEnumValue("DstColor", 7473533985030373306u, 4);
enumValue = type.addEnumValue("OneMinusDstColor", 13349783845657362766u, 5);
enumValue = type.addEnumValue("SrcAlpha", 7279042487094499998u, 6);
enumValue = type.addEnumValue("OneMinusSrcAlpha", 10873798645618967378u, 7);
enumValue = type.addEnumValue("DstAlpha", 3638315451497627405u, 8);
enumValue = type.addEnumValue("OneMinusDstAlpha", 11193193720308034817u, 9);
enumValue = type.addEnumValue("ConstantColor", 15927831585855134043u, 10);
enumValue = type.addEnumValue("OneMinusConstantColor", 15648017966302123871u, 11);
enumValue = type.addEnumValue("ConstantAlpha", 246207556337099080u, 12);
enumValue = type.addEnumValue("OneMinusConstantAlpha", 11885865238674929644u, 13);
}
return &type; }
namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_10684716730739981600u = *rfk::getEnum<BlendType>(); }
template <> rfk::Enum const* rfk::getEnum<TextureType>() noexcept
{
static bool initialized = false;
static rfk::Enum type("TextureType", 1469356299016962604u, rfk::getArchetype<int>());
if (!initialized) {
initialized = true;
rfk::EnumValue* enumValue = nullptr;
type.setEnumValuesCapacity(2);
enumValue = type.addEnumValue("Default", 16096079758550944345u, 0);
enumValue = type.addEnumValue("Heightmap", 2530056802210335799u, 1);
}
return &type; }
namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_1469356299016962604u = *rfk::getEnum<TextureType>(); }
template <> rfk::Enum const* rfk::getEnum<DepthType>() noexcept
{
static bool initialized = false;
static rfk::Enum type("DepthType", 2155720045105603770u, rfk::getArchetype<int>());
if (!initialized) {
initialized = true;
rfk::EnumValue* enumValue = nullptr;
type.setEnumValuesCapacity(8);
enumValue = type.addEnumValue("Never", 5021203232326339496u, 0);
enumValue = type.addEnumValue("Less", 13285040111643951031u, 1);
enumValue = type.addEnumValue("Equal", 3834143011815406898u, 2);
enumValue = type.addEnumValue("LessEqual", 8513454352698843133u, 3);
enumValue = type.addEnumValue("Greater", 15695988502959682692u, 4);
enumValue = type.addEnumValue("NotEqual", 13956109531584779985u, 5);
enumValue = type.addEnumValue("GreaterEqual", 1345347424818286844u, 6);
enumValue = type.addEnumValue("Always", 2809980761389189675u, 7);
}
return &type; }
namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_2155720045105603770u = *rfk::getEnum<DepthType>(); }
template <> rfk::Enum const* rfk::getEnum<StencilOperationType>() noexcept
{
static bool initialized = false;
static rfk::Enum type("StencilOperationType", 4329759726231724296u, rfk::getArchetype<int>());
if (!initialized) {
initialized = true;
rfk::EnumValue* enumValue = nullptr;
type.setEnumValuesCapacity(3);
enumValue = type.addEnumValue("Set", 13171209841654809432u, 0);
enumValue = type.addEnumValue("ResetNotEqual", 8556378855153469056u, 1);
enumValue = type.addEnumValue("ResetAlways", 2249997453837997558u, 2);
}
return &type; }
namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_4329759726231724296u = *rfk::getEnum<StencilOperationType>(); }
template <> rfk::Enum const* rfk::getEnum<WindingOrder>() noexcept
{
static bool initialized = false;
static rfk::Enum type("WindingOrder", 16906535053678320275u, rfk::getArchetype<int>());
if (!initialized) {
initialized = true;
rfk::EnumValue* enumValue = nullptr;
type.setEnumValuesCapacity(2);
enumValue = type.addEnumValue("ClockWise", 10294138846565727201u, 0);
enumValue = type.addEnumValue("CounterClockWise", 10930050431606043363u, 1);
}
return &type; }
namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_16906535053678320275u = *rfk::getEnum<WindingOrder>(); }
template <> rfk::Enum const* rfk::getEnum<ShaderType>() noexcept
{
static bool initialized = false;
static rfk::Enum type("ShaderType", 8939940529319396080u, rfk::getArchetype<int>());
if (!initialized) {
initialized = true;
rfk::EnumValue* enumValue = nullptr;
type.setEnumValuesCapacity(3);
enumValue = type.addEnumValue("VertexShader", 11429502142427327383u, 0);
enumValue = type.addEnumValue("FragmentShader", 1146633447898575857u, 1);
enumValue = type.addEnumValue("ComputeShader", 8311518917626496796u, 2);
}
return &type; }
namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_8939940529319396080u = *rfk::getEnum<ShaderType>(); }
template <> rfk::Enum const* rfk::getEnum<Key>() noexcept
{
static bool initialized = false;
static rfk::Enum type("Key", 6248865675226647868u, rfk::getArchetype<int>());
if (!initialized) {
initialized = true;
rfk::EnumValue* enumValue = nullptr;
type.setEnumValuesCapacity(58);
enumValue = type.addEnumValue("KeySpace", 5651318088130929055u, 32);
enumValue = type.addEnumValue("Key0", 7104935663355106585u, 48);
enumValue = type.addEnumValue("Key1", 7104934563843478374u, 49);
enumValue = type.addEnumValue("Key2", 7104933464331850163u, 50);
enumValue = type.addEnumValue("Key3", 7104932364820221952u, 51);
enumValue = type.addEnumValue("Key4", 7104940061401619429u, 52);
enumValue = type.addEnumValue("Key5", 7104938961889991218u, 53);
enumValue = type.addEnumValue("Key6", 7104937862378363007u, 54);
enumValue = type.addEnumValue("Key7", 7104936762866734796u, 55);
enumValue = type.addEnumValue("Key8", 7104944459448132273u, 56);
enumValue = type.addEnumValue("Key9", 7104943359936504062u, 57);
enumValue = type.addEnumValue("KeyA", 7105057709145838006u, 65);
enumValue = type.addEnumValue("KeyB", 7105056609634209795u, 66);
enumValue = type.addEnumValue("KeyC", 7105055510122581584u, 67);
enumValue = type.addEnumValue("KeyD", 7105063206703979061u, 68);
enumValue = type.addEnumValue("KeyE", 7105062107192350850u, 69);
enumValue = type.addEnumValue("KeyF", 7105061007680722639u, 70);
enumValue = type.addEnumValue("KeyG", 7105059908169094428u, 71);
enumValue = type.addEnumValue("KeyH", 7105067604750491905u, 72);
enumValue = type.addEnumValue("KeyI", 7105066505238863694u, 73);
enumValue = type.addEnumValue("KeyJ", 7105065405727235483u, 74);
enumValue = type.addEnumValue("KeyK", 7105064306215607272u, 75);
enumValue = type.addEnumValue("KeyL", 7105072002797004749u, 76);
enumValue = type.addEnumValue("KeyM", 7105070903285376538u, 77);
enumValue = type.addEnumValue("KeyN", 7105069803773748327u, 78);
enumValue = type.addEnumValue("KeyO", 7105068704262120116u, 79);
enumValue = type.addEnumValue("KeyP", 7105041216471414841u, 80);
enumValue = type.addEnumValue("KeyQ", 7105040116959786630u, 81);
enumValue = type.addEnumValue("KeyR", 7105039017448158419u, 82);
enumValue = type.addEnumValue("KeyS", 7105037917936530208u, 83);
enumValue = type.addEnumValue("KeyT", 7105045614517927685u, 84);
enumValue = type.addEnumValue("KeyU", 7105044515006299474u, 85);
enumValue = type.addEnumValue("KeyV", 7105043415494671263u, 86);
enumValue = type.addEnumValue("KeyW", 7105042315983043052u, 87);
enumValue = type.addEnumValue("KeyX", 7105050012564440529u, 88);
enumValue = type.addEnumValue("KeyY", 7105048913052812318u, 89);
enumValue = type.addEnumValue("KeyZ", 7105047813541184107u, 90);
enumValue = type.addEnumValue("KeyTab", 9768112940582220618u, 258);
enumValue = type.addEnumValue("KeyF1", 15415997317357523354u, 290);
enumValue = type.addEnumValue("KeyF2", 15415996217845895143u, 291);
enumValue = type.addEnumValue("KeyF3", 15415995118334266932u, 292);
enumValue = type.addEnumValue("KeyF4", 15415994018822638721u, 293);
enumValue = type.addEnumValue("KeyF5", 15415992919311010510u, 294);
enumValue = type.addEnumValue("KeyF6", 15415991819799382299u, 295);
enumValue = type.addEnumValue("KeyF7", 15415990720287754088u, 296);
enumValue = type.addEnumValue("KeyF8", 15415989620776125877u, 297);
enumValue = type.addEnumValue("KeyF9", 15415988521264497666u, 298);
enumValue = type.addEnumValue("KeyF10", 13887894887515102u, 299);
enumValue = type.addEnumValue("KeyF11", 13888994399143313u, 300);
enumValue = type.addEnumValue("KeyF12", 13885695864258680u, 301);
enumValue = type.addEnumValue("KeyEscape", 1249037196286773812u, 256);
enumValue = type.addEnumValue("KeyDelete", 3672620855681699312u, 261);
enumValue = type.addEnumValue("KeyRight", 554971525204525623u, 262);
enumValue = type.addEnumValue("KeyLeft", 3269711060223514954u, 263);
enumValue = type.addEnumValue("KeyDown", 12581239460013411375u, 264);
enumValue = type.addEnumValue("KeyUp", 15401544237007528134u, 265);
enumValue = type.addEnumValue("KeyShift", 3788917732177651857u, 340);
enumValue = type.addEnumValue("KeyLeftControl", 1830905774986876303u, 341);
}
return &type; }
namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_6248865675226647868u = *rfk::getEnum<Key>(); }
template <> rfk::Enum const* rfk::getEnum<MouseButton>() noexcept
{
static bool initialized = false;
static rfk::Enum type("MouseButton", 17352997537815214736u, rfk::getArchetype<int>());
if (!initialized) {
initialized = true;
rfk::EnumValue* enumValue = nullptr;
type.setEnumValuesCapacity(3);
enumValue = type.addEnumValue("MouseButtonLeft", 6534413019849402026u, 0);
enumValue = type.addEnumValue("MouseButtonRight", 6953304312221125399u, 1);
enumValue = type.addEnumValue("MouseButtonMiddle", 11925931373657146254u, 2);
}
return &type; }
namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_17352997537815214736u = *rfk::getEnum<MouseButton>(); }
template <> rfk::Function const* rfk::getFunction<static_cast<glm::mat<4, 4, float, glm::packed_highp>(*)(const Vector3 &, const Vector3 &, const Vector3 &)>(&LookAt)>() noexcept {
static bool initialized = false;
static rfk::Function function("LookAt", 3258074355963504694u, rfk::getType<glm::mat<4, 4, float, glm::packed_highp>>(), new rfk::NonMemberFunction<glm::mat4 (const Vector3 &, const Vector3 &, const Vector3 &)>(&LookAt), static_cast<rfk::EFunctionFlags>(2));
if (!initialized) {
initialized = true;
function.setParametersCapacity(3);
function.addParameter("eye", 0u, rfk::getType<const Vector3 &>());
function.addParameter("center", 0u, rfk::getType<const Vector3 &>());
function.addParameter("up", 0u, rfk::getType<const Vector3 &>());
;
}return &function; }
namespace rfk::generated { static rfk::DefaultEntityRegisterer const registerer3258074355963504694u = *rfk::getFunction<static_cast<glm::mat<4, 4, float, glm::packed_highp>(*)(const Vector3 &, const Vector3 &, const Vector3 &)>(&LookAt)>(); }
template <> rfk::Function const* rfk::getFunction<static_cast<std::basic_string<char>(*)(const RenderingPath &)>(&ToString)>() noexcept {
static bool initialized = false;
static rfk::Function function("ToString", 10791883510675771287u, rfk::getType<std::basic_string<char>>(), new rfk::NonMemberFunction<std::string (const RenderingPath &)>(&ToString), static_cast<rfk::EFunctionFlags>(2));
if (!initialized) {
initialized = true;
function.setParametersCapacity(1);
function.addParameter("value", 0u, rfk::getType<const RenderingPath &>());
;
}return &function; }
namespace rfk::generated { static rfk::DefaultEntityRegisterer const registerer10791883510675771287u = *rfk::getFunction<static_cast<std::basic_string<char>(*)(const RenderingPath &)>(&ToString)>(); }
template <> rfk::Function const* rfk::getFunction<static_cast<std::basic_string<char>(*)(const CameraType &)>(&ToString)>() noexcept {
static bool initialized = false;
static rfk::Function function("ToString", 511960883423803161u, rfk::getType<std::basic_string<char>>(), new rfk::NonMemberFunction<std::string (const CameraType &)>(&ToString), static_cast<rfk::EFunctionFlags>(2));
if (!initialized) {
initialized = true;
function.setParametersCapacity(1);
function.addParameter("value", 0u, rfk::getType<const CameraType &>());
;
}return &function; }
namespace rfk::generated { static rfk::DefaultEntityRegisterer const registerer511960883423803161u = *rfk::getFunction<static_cast<std::basic_string<char>(*)(const CameraType &)>(&ToString)>(); }
template <> rfk::Function const* rfk::getFunction<static_cast<RenderingPath(*)(const std::string &)>(&FromString)>() noexcept {
static bool initialized = false;
static rfk::Function function("FromString", 4780674481163204375u, rfk::getType<RenderingPath>(), new rfk::NonMemberFunction<RenderingPath (const std::string &)>(&FromString), static_cast<rfk::EFunctionFlags>(2));
if (!initialized) {
initialized = true;
function.setParametersCapacity(1);
function.addParameter("input", 0u, rfk::getType<const std::string &>());
;
}return &function; }
namespace rfk::generated { static rfk::DefaultEntityRegisterer const registerer4780674481163204375u = *rfk::getFunction<static_cast<RenderingPath(*)(const std::string &)>(&FromString)>(); }
template <> rfk::Function const* rfk::getFunction<static_cast<CameraType(*)(const std::string &)>(&FromString)>() noexcept {
static bool initialized = false;
static rfk::Function function("FromString", 4780674481163204375u, rfk::getType<CameraType>(), new rfk::NonMemberFunction<CameraType (const std::string &)>(&FromString), static_cast<rfk::EFunctionFlags>(2));
if (!initialized) {
initialized = true;
function.setParametersCapacity(1);
function.addParameter("input", 0u, rfk::getType<const std::string &>());
;
}return &function; }
namespace rfk::generated { static rfk::DefaultEntityRegisterer const registerer4780674481163204375u = *rfk::getFunction<static_cast<CameraType(*)(const std::string &)>(&FromString)>(); }
template <> rfk::Function const* rfk::getFunction<static_cast<std::basic_string<char>(*)()>(&FindSolutionPath)>() noexcept {
static bool initialized = false;
static rfk::Function function("FindSolutionPath", 7406603909466443062u, rfk::getType<std::basic_string<char>>(), new rfk::NonMemberFunction<std::string ()>(&FindSolutionPath), static_cast<rfk::EFunctionFlags>(2));
if (!initialized) {
initialized = true;
}return &function; }
namespace rfk::generated { static rfk::DefaultEntityRegisterer const registerer7406603909466443062u = *rfk::getFunction<static_cast<std::basic_string<char>(*)()>(&FindSolutionPath)>(); }
template <> rfk::Function const* rfk::getFunction<static_cast<std::basic_string<char>(*)()>(&getProjectRoot)>() noexcept {
static bool initialized = false;
static rfk::Function function("getProjectRoot", 6850348136504988476u, rfk::getType<std::basic_string<char>>(), new rfk::NonMemberFunction<std::string ()>(&getProjectRoot), static_cast<rfk::EFunctionFlags>(2));
if (!initialized) {
initialized = true;
}return &function; }
namespace rfk::generated { static rfk::DefaultEntityRegisterer const registerer6850348136504988476u = *rfk::getFunction<static_cast<std::basic_string<char>(*)()>(&getProjectRoot)>(); }
template <> rfk::Function const* rfk::getFunction<static_cast<std::basic_string<char>(*)()>(&getMSBuildPath)>() noexcept {
static bool initialized = false;
static rfk::Function function("getMSBuildPath", 14190331423545695390u, rfk::getType<std::basic_string<char>>(), new rfk::NonMemberFunction<std::string ()>(&getMSBuildPath), static_cast<rfk::EFunctionFlags>(2));
if (!initialized) {
initialized = true;
}return &function; }
namespace rfk::generated { static rfk::DefaultEntityRegisterer const registerer14190331423545695390u = *rfk::getFunction<static_cast<std::basic_string<char>(*)()>(&getMSBuildPath)>(); }
template <> rfk::Function const* rfk::getFunction<static_cast<std::basic_string<char>(*)(const std::type_info &)>(&getClassName)>() noexcept {
static bool initialized = false;
static rfk::Function function("getClassName", 10839305348980006990u, rfk::getType<std::basic_string<char>>(), new rfk::NonMemberFunction<std::string (const std::type_info &)>(&getClassName), static_cast<rfk::EFunctionFlags>(2));
if (!initialized) {
initialized = true;
function.setParametersCapacity(1);
function.addParameter("typeInfo", 0u, rfk::getType<const std::type_info &>());
;
}return &function; }
namespace rfk::generated { static rfk::DefaultEntityRegisterer const registerer10839305348980006990u = *rfk::getFunction<static_cast<std::basic_string<char>(*)(const std::type_info &)>(&getClassName)>(); }

