#pragma once

#include "../FileWatcher.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_10852759882766763751u = FileWatcher::staticGetArchetype(); }

rfk::Class const& FileWatcher::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Class type("FileWatcher", 10852759882766763751u, sizeof(FileWatcher), 1);
if (!initialized) {
initialized = true;
FileWatcher::_rfk_registerChildClass<FileWatcher>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<FileWatcher>>(),new rfk::NonMemberFunction<rfk::SharedPtr<FileWatcher>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<FileWatcher>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<FileWatcher>>(),new rfk::NonMemberFunction<rfk::UniquePtr<FileWatcher>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<FileWatcher>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<FileWatcher>() noexcept { return &FileWatcher::staticGetArchetype(); }


