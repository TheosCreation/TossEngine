/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : LayerManager.h
Description : Provides a dynamic layer management system using bit flags for filtering, collision masks, etc.
              Automatically allocates and reuses available layer bits. Supports JSON serialization.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Utils.h"
#include "Serializable.h"

using LayerBit = unsigned short;

/**
 * @class LayerManager
 * @brief Singleton class for managing named bitmask layers in the engine.
 *        Used for filtering (e.g., physics, rendering) via bit flags.
 */
class TOSSENGINE_API LayerManager : public Serializable
{
public:
    /**
     * @brief Retrieves the singleton instance of LayerManager.
     * @return Reference to the singleton.
     */
    static LayerManager& GetInstance() {
        static LayerManager instance;
        return instance;
    }

    // Delete copy constructor and assignment operator to enforce singleton pattern.
    LayerManager(const LayerManager&) = delete;
    LayerManager& operator=(const LayerManager&) = delete;

    /**
     * @brief Retrieves or creates a layer bit associated with the given name.
     * @param name The name of the layer.
     * @return The assigned bit value.
     * @throws std::runtime_error if the layer count exceeds the maximum available bits.
     */
    LayerBit GetLayer(const std::string& name) {
        auto it = m_layerMap.find(name);
        if (it != m_layerMap.end())
            return it->second;

        // Allocate new bit index
        int bitIndex;
        if (!m_freeBitIndices.empty()) {
            bitIndex = m_freeBitIndices.back();
            m_freeBitIndices.pop_back();
        }
        else {
            constexpr int MAX_BITS = sizeof(LayerBit) * 8; // 16
            if (m_nextBitIndex >= MAX_BITS) {
                throw std::runtime_error("Maximum number of layers reached.");
            }
            bitIndex = m_nextBitIndex++;
        }

        LayerBit newBit = static_cast<LayerBit>(1u << bitIndex);
        m_layerMap[name] = newBit;
        return newBit;
    }

    /**
     * @brief Retrieves the full layer name-to-bit map.
     * @return Reference to the internal layer map.
     */
    const std::unordered_map<std::string, LayerBit>& GetLayers() const {
        return m_layerMap;
    }

    /**
     * @brief Checks if a layer with the given name exists.
     * @param name The name of the layer to query.
     * @return True if the layer exists.
     */
    bool HasLayer(const std::string& name) const {
        return m_layerMap.find(name) != m_layerMap.end();
    }

    /**
     * @brief Removes a layer by name.
     *        Frees its bit index for reuse.
     * @param name The name of the layer to remove.
     * @return True if removed successfully; false if not found or attempting to remove "Default".
     */
    bool RemoveLayer(const std::string& name) {
        if (name == "Default") {
            return false; // Always keep the Default layer.
        }

        auto it = m_layerMap.find(name);
        if (it != m_layerMap.end()) {
            LayerBit bit = it->second;

            // Derive bit index from bit value
            unsigned int bitIndex = 0;
            while (bitIndex < 32 && (1u << bitIndex) != bit) {
                ++bitIndex;
            }

            m_layerMap.erase(it);
            m_freeBitIndices.push_back(bitIndex);
            return true;
        }

        return false;
    }

    /**
     * @brief Serializes layer state to JSON.
     * @return A JSON object representing the layer manager's state.
     */
    json serialize() const override {
        json data;
        data["nextBitIndex"] = m_nextBitIndex;

        for (const auto& pair : m_layerMap) {
            data["layers"][pair.first] = pair.second;
        }

        data["freeBitIndices"] = m_freeBitIndices;
        return data;
    }

    /**
     * @brief Deserializes layer state from JSON.
     * @param data The JSON object to load from.
     */
    void deserialize(const json& data) override {
        m_layerMap.clear();
        m_freeBitIndices.clear();

        if (data.contains("nextBitIndex")) {
            m_nextBitIndex = data["nextBitIndex"].get<unsigned int>();
        }

        if (data.contains("layers")) {
            for (auto& [name, bit] : data["layers"].items()) {
                m_layerMap[name] = bit.get<LayerBit>();
            }
        }

        if (data.contains("freeBitIndices")) {
            m_freeBitIndices = data["freeBitIndices"].get<std::vector<unsigned int>>();
        }

        // Always ensure Default layer exists
        if (m_layerMap.find("Default") == m_layerMap.end()) {
            GetLayer("Default");
        }
    }

private:
    /**
     * @brief Private constructor. Ensures "Default" layer is always registered on startup.
     */
    LayerManager() {
        GetLayer("Default");
    }

    std::unordered_map<std::string, LayerBit> m_layerMap; //!< Map of layer name to bitmask.
    unsigned int m_nextBitIndex = 0;                      //!< Next available bit index.
    std::vector<unsigned int> m_freeBitIndices;           //!< Stack of reusable freed bit indices.
};
