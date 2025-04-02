#pragma once

#include "../Math.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

template <> rfk::Function const* rfk::getFunction<static_cast<void(*)()>(&initRandomSeed)>() noexcept {
static bool initialized = false;
static rfk::Function function("initRandomSeed", 15057944669220928335u, rfk::getType<void>(), new rfk::NonMemberFunction<void ()>(&initRandomSeed), static_cast<rfk::EFunctionFlags>(2));
if (!initialized) {
initialized = true;
}return &function; }
namespace rfk::generated { static rfk::DefaultEntityRegisterer const registerer15057944669220928335u = *rfk::getFunction<static_cast<void(*)()>(&initRandomSeed)>(); }
template <> rfk::Function const* rfk::getFunction<static_cast<float(*)(float)>(&randomNumber)>() noexcept {
static bool initialized = false;
static rfk::Function function("randomNumber", 6085631163347581694u, rfk::getType<float>(), new rfk::NonMemberFunction<float (float)>(&randomNumber), static_cast<rfk::EFunctionFlags>(2));
if (!initialized) {
initialized = true;
function.setParametersCapacity(1);
function.addParameter("max", 0u, rfk::getType<float>());
;
}return &function; }
namespace rfk::generated { static rfk::DefaultEntityRegisterer const registerer6085631163347581694u = *rfk::getFunction<static_cast<float(*)(float)>(&randomNumber)>(); }
template <> rfk::Function const* rfk::getFunction<static_cast<float(*)(float, float)>(&randomRange)>() noexcept {
static bool initialized = false;
static rfk::Function function("randomRange", 7138727628727291835u, rfk::getType<float>(), new rfk::NonMemberFunction<float (float, float)>(&randomRange), static_cast<rfk::EFunctionFlags>(2));
if (!initialized) {
initialized = true;
function.setParametersCapacity(2);
function.addParameter("min", 0u, rfk::getType<float>());
function.addParameter("max", 0u, rfk::getType<float>());
;
}return &function; }
namespace rfk::generated { static rfk::DefaultEntityRegisterer const registerer7138727628727291835u = *rfk::getFunction<static_cast<float(*)(float, float)>(&randomRange)>(); }

