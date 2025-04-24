#pragma once
#include "Utils.h"
#include "Serializable.h"

using LayerBit = unsigned short;

class TOSSENGINE_API LayerManager : public Serializable
{
public:
    // Get the singleton instance.
    static LayerManager& GetInstance() {
        static LayerManager instance;
        return instance;
    }

    // Delete copy constructor and assignment operator.
    LayerManager(const LayerManager&) = delete;
    LayerManager& operator=(const LayerManager&) = delete;

    LayerBit GetLayer(const std::string& name) {
        auto it = m_layerMap.find(name);
        if (it != m_layerMap.end())
            return it->second;

        // pick a bit index
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

        // do the shift in an unsigned int, then cast down
        LayerBit newBit = static_cast<LayerBit>((1u << bitIndex));
        m_layerMap[name] = newBit;
        return newBit;
    }

    const std::unordered_map<std::string, LayerBit>& GetLayers() const {
        return m_layerMap;
    }

    // Check if a layer exists.
    bool HasLayer(const std::string& name) const {
        return m_layerMap.find(name) != m_layerMap.end();
    }

    bool RemoveLayer(const std::string& name) {
        if (name == "Default") {
            return false; // Do not remove the default layer.
        }
        auto it = m_layerMap.find(name);
        if (it != m_layerMap.end()) {
            // Compute the bit index from the layer's bit value.
            LayerBit bit = it->second;
            unsigned int bitIndex = 0;
            while (bitIndex < 32 && (1u << bitIndex) != bit) {
                ++bitIndex;
            }
            m_layerMap.erase(it);
            // Add the freed bit index to the free list.
            m_freeBitIndices.push_back(bitIndex);
            return true;
        }
        return false;
    }

    // Serialize to JSON
    json serialize() const override
    {
        json data;
        data["nextBitIndex"] = m_nextBitIndex;
        // Save the mapping of layer names to bits.
        for (const auto& pair : m_layerMap) {
            data["layers"][pair.first] = pair.second;
        }
        // Save the free indices list.
        data["freeBitIndices"] = m_freeBitIndices;
        return data;
    }

    // Deserialize from JSON
    void deserialize(const json& data) override
    {
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
        // Ensure Default layer always exists.
        if (m_layerMap.find("Default") == m_layerMap.end()) {
            GetLayer("Default");
        }
    }

private:
    // Private constructor for singleton.
    LayerManager()
    {
        GetLayer("Default");
    }

    std::unordered_map<std::string, LayerBit> m_layerMap;
    unsigned int m_nextBitIndex = 0;
    std::vector<unsigned int> m_freeBitIndices;
};