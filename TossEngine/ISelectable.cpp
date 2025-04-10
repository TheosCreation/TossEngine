#include "ISelectable.h"
#include "LayerManager.h"
#include "Resource.h"

bool ISelectable::FloatSliderField(const std::string& name, float& value, float speed, float min, float max)
{
    if (ImGui::DragFloat(name.c_str(), &value, speed, min, max))
    {
        return true;
    }
    return false;
}


static std::string GetSelectedLayersString(const std::vector<std::string>& selectedLayers) {
    if (selectedLayers.empty()) {
        return "Default";
    }
    std::string result;
    for (const auto& name : selectedLayers) {
        result += name + ", ";
    }
    if (!result.empty()) {
        // Remove trailing ", "
        result.pop_back();
        result.pop_back();
    }
    return result;
}

bool ISelectable::LayerDropdownField(const string& name, vector<string>& value)
{
    if (value.empty()) {
        value.push_back("Default");
    }

    // Get available layers from the LayerManager.
    LayerManager& layerManager = LayerManager::GetInstance();
    const auto& layers = layerManager.GetLayers();

    // Build a vector copy of available layer names.
    std::vector<std::string> layerNames;
    for (const auto& pair : layers) {
        layerNames.push_back(pair.first);
    }

    // Build a preview string from the currently selected layers.
    std::string previewText = GetSelectedLayersString(value);
    if (ImGui::BeginCombo(name.c_str(), previewText.c_str())) {
        // Iterate over each available layer.
        for (const auto& layer : layerNames) {
            bool isSelected = (std::find(value.begin(), value.end(), layer) != value.end());

            // Display each layer as a selectable item.
            if (ImGui::Selectable(layer.c_str(), isSelected)) {
                if (isSelected) {
                    // Remove the layer if it was already selected.
                    value.erase(std::remove(value.begin(), value.end(), layer), value.end());
                }
                else {
                    // Add the layer if it wasn't selected.
                    value.push_back(layer);
                }
            }
        }
        ImGui::EndCombo();
        return true;
    }
    return false;
}

bool ISelectable::BoolCheckboxField(const string& name, bool& value)
{
    if(ImGui::Checkbox(name.c_str(), &value))
    {
        return true;
    }
    return false;
}
