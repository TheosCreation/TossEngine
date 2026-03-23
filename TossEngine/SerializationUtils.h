#pragma once
#include "Utils.h"

/**
 * @brief Serializes a GameObjectPtr to JSON.
 */
inline void to_json(json& j, GameObjectPtr const& gameObject);

/**
 * @brief Deserializes a GameObjectPtr from JSON.
 */
inline void from_json(json const& j, GameObjectPtr& gameObject);