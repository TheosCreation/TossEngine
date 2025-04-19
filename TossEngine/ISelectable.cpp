#include "ISelectable.h"
#include "LayerManager.h"
#include "Material.h"
#include "TossEngine.h"

void ISelectable::GameObjectAssignableField(GameObject*& _gameObject, const string& fieldName)
{
    GameObjectManager* gameObjectManager = TossEngine::GetInstance().getGameObjectManager();

    string label = (_gameObject)
        ? (fieldName + ": " + _gameObject->name)
        : (fieldName + ": None");

    if (ImGui::BeginTable((fieldName + "Table").c_str(), 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::Selectable(label.c_str());

        ImGui::TableSetColumnIndex(1);
        if (ImGui::SmallButton("+"))
        {
            ImGui::OpenPopup((fieldName + "Dropdown").c_str());
        }

        if (ImGui::BeginPopup((fieldName + "Dropdown").c_str()))
        {
            bool noneSelected = (_gameObject == nullptr);
            if (ImGui::Selectable("None", noneSelected))
            {
                _gameObject = nullptr;
            }

            for (const auto& [id, gameObject] : gameObjectManager->m_gameObjects)
            {
                if (!gameObject) continue;

                bool isSelected = (_gameObject && _gameObject->getId() == id);
                if (ImGui::Selectable(gameObject->name.c_str(), isSelected))
                {
                    _gameObject = gameObject;
                }
            }
            ImGui::EndPopup();
        }
        ImGui::EndTable();
    }
}

bool ISelectable::FloatSliderField(const std::string& name, float& value, float speed, float min, float max)
{
    if (ImGui::DragFloat(name.c_str(), &value, speed, min, max))
    {
        return true;
    }
    return false;
}

bool ISelectable::IntSliderField(const string& name, int& value, int speed, int min, int max)
{
    if (ImGui::DragInt(name.c_str(), &value, speed, min, max))
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
