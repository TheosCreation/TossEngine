/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : ISelectable.h
Description : Provides an interface for objects that can be selected and displayed in the TossEngine editor inspector.
              Also provides various helper functions for ImGui-based field rendering.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Utils.h"
#include "ResourceManager.h"

class GameObject;

/**
 * @class ISelectable
 * @brief Interface for any object that can be selected in the editor and shown in the Inspector window.
 */
class TOSSENGINE_API ISelectable
{
public:
    // ----- Virtual Inspector Methods -----

    /**
     * @brief Called to render the object's custom fields in the Inspector.
     */
    virtual void OnInspectorGUI() {}

    /**
     * @brief Called when the object is selected.
     */
    virtual void OnSelect() {}

    /**
     * @brief Called when the object is deselected.
     */
    virtual void OnDeSelect() {}

    /**
     * @brief Called when the object should be deleted.
     * @param deleteSelf If true, the object should delete itself.
     * @return True if the object was successfully deleted.
     */
    virtual bool Delete(bool deleteSelf = true) { return false; }

    // ----- Static Helper Methods -----

    /**
     * @brief Renders a selectable GameObject assignment field.
     * @param _gameObject Reference to the GameObject pointer to modify.
     * @param fieldName The label to display in the Inspector.
     */
    static bool GameObjectAssignableField(GameObjectPtr& _gameObject, const std::string& fieldName);

    /**
     * @brief Renders a resource assignment field using a table layout.
     * @tparam T The resource type (must inherit from Resource).
     * @param resourcePtr Reference to the resource pointer to modify.
     * @param fieldName The label to display in the Inspector.
     * @return True if the resource was changed.
     */
    template<typename T>
    bool ResourceAssignableField(std::shared_ptr<T>& resourcePtr, const std::string& fieldName)
    {
        static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

        ResourceManager& rm = ResourceManager::GetInstance();
        bool changed = false;
        std::string label = resourcePtr
            ? fieldName + ": " + resourcePtr->getUniqueID()
            : fieldName + ": None";

        if (ImGui::BeginTable((fieldName + "Table").c_str(), 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            if (ImGui::Selectable(label.c_str()))
            {
                rm.SetSelectedResource(resourcePtr);
            }

            ImGui::TableSetColumnIndex(1);
            if (ImGui::SmallButton("+"))
                ImGui::OpenPopup((fieldName + "Dropdown").c_str());

            if (ImGui::BeginPopup((fieldName + "Dropdown").c_str()))
            {
                // "None" option
                if (ImGui::Selectable("None", resourcePtr == nullptr))
                {
                    if (resourcePtr)
                    {
                        resourcePtr.reset();
                        rm.SetSelectedResource(nullptr);
                        changed = true;
                    }
                }

                // List all available resources of type T
                auto& all = rm.GetAllResources();
                for (auto& [id, res] : all)
                {
                    if (auto casted = std::dynamic_pointer_cast<T>(res))
                    {
                        bool isSel = (resourcePtr && resourcePtr->getUniqueID() == id);
                        if (ImGui::Selectable(id.c_str(), isSel))
                        {
                            if (!isSel)
                            {
                                resourcePtr = casted;
                                rm.SetSelectedResource(resourcePtr);
                                changed = true;
                            }
                        }
                    }
                }
                ImGui::EndPopup();
            }

            ImGui::EndTable();
        }

        return changed;
    }

    /**
     * @brief Renders a dropdown selection field for selecting a resource.
     * @tparam T The resource type (must inherit from Resource).
     * @param resourcePtr Reference to the resource pointer to modify.
     * @param fieldName The label to display in the Inspector.
     * @return True if a new resource was selected.
     */
    template<typename T>
    bool ResourceDropdownField(std::shared_ptr<T>& resourcePtr, const std::string& fieldName)
    {
        static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

        ResourceManager& resourceManager = ResourceManager::GetInstance();
        std::string previewLabel = (resourcePtr) ? resourcePtr->getUniqueID() : "None";

        if (ImGui::BeginCombo(fieldName.c_str(), previewLabel.c_str()))
        {
            bool didSelect = false;

            // "None" option
            bool noneSelected = (resourcePtr == nullptr);
            if (ImGui::Selectable("None", noneSelected))
            {
                resourcePtr = nullptr;
                resourceManager.SetSelectedResource(nullptr);
            }

            // List all resources of the correct type
            auto& resources = resourceManager.GetAllResources();
            for (const auto& [id, resource] : resources)
            {
                auto casted = std::dynamic_pointer_cast<T>(resource);
                if (casted)
                {
                    bool isSelected = (resourcePtr && resourcePtr->getUniqueID() == id);
                    if (ImGui::Selectable(id.c_str(), isSelected))
                    {
                        resourcePtr = casted;
                        resourceManager.SetSelectedResource(resourcePtr);
                        didSelect = true;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
            return didSelect;
        }
        return false;
    }

    /**
     * @brief Displays a file selection table.
     * @param tableID The ID of the ImGui table.
     * @param rowLabels Labels for each row.
     * @param filePaths Selected file paths to modify.
     * @param filter File extension filter.
     * @return True if the selection changed.
     */
    static bool FileSelectionTableField(const char* tableID, const std::vector<std::string>& rowLabels, std::vector<std::string>& filePaths, const char* filter);

    /**
     * @brief Displays a float slider field.
     * @param name Label for the field.
     * @param value Reference to the float to modify.
     * @param speed Slider sensitivity.
     * @param min Minimum slider value.
     * @param max Maximum slider value.
     * @return True if the value changed.
     */
    static bool FloatSliderField(const std::string& name, float& value, float speed = 1.0f, float min = 0.0f, float max = 0.0f);

    /**
     * @brief Displays an int slider field.
     * @param name Label for the field.
     * @param value Reference to the int to modify.
     * @param speed Slider sensitivity.
     * @param min Minimum slider value.
     * @param max Maximum slider value.
     * @return True if the value changed.
     */
    static bool IntSliderField(const std::string& name, int& value, int speed = 1, int min = 0, int max = 0);

    /**
     * @brief Displays a dropdown for selecting layers.
     * @param name Label for the field.
     * @param value Vector of selected layers.
     * @return True if the selection changed.
     */
    static bool LayerDropdownField(const std::string& name, std::vector<std::string>& value);

    /**
     * @brief Displays a boolean checkbox field.
     * @param name Label for the field.
     * @param value Reference to the bool to modify.
     * @return True if the value changed.
     */
    static bool BoolCheckboxField(const std::string& name, bool& value);

    template<typename T>
    void VectorField(std::vector<T>& vec, const std::string& label)
    {
        if (ImGui::CollapsingHeader((label + " List").c_str()))
        {
            int toRemoveIndex = -1;

            ImGui::Columns(2, nullptr, false); // Start 2 columns for all items

            for (size_t i = 0; i < vec.size(); ++i)
            {
                ImGui::PushID(static_cast<int>(i));

                std::string elementLabel = label + " " + std::to_string(i);
                DrawVectorElementField(elementLabel, vec[i]); // Left side: the field

                ImGui::NextColumn(); // Move to right column

                if (ImGui::SmallButton("Remove"))
                {
                    toRemoveIndex = static_cast<int>(i);
                }

                ImGui::NextColumn(); // Important! Go back to first column for next item

                ImGui::PopID();
            }

            ImGui::Columns(1); // End columns cleanly

            if (toRemoveIndex >= 0 && toRemoveIndex < static_cast<int>(vec.size()))
            {
                vec.erase(vec.begin() + toRemoveIndex);
            }

            if (ImGui::Button(("Add " + label).c_str()))
            {
                vec.emplace_back(T{});
            }
        }
    }
};
// just hard coded this so we route it to the right place
inline bool DrawVectorElementField(const std::string& label, int& val) {
    return ISelectable::IntSliderField(label, val);
}

inline bool DrawVectorElementField(const std::string& label, float& val) {
    return ISelectable::FloatSliderField(label, val);
}

inline bool DrawVectorElementField(const std::string& label, bool& val) {
    return ISelectable::BoolCheckboxField(label, val);
}

template <typename T>
inline bool DrawVectorElementField(const std::string& label, std::shared_ptr<T>& val) {
    return ISelectable::ResourceAssignableField<T>(val, label);
}

inline bool DrawVectorElementField(const std::string& label, GameObjectPtr& val) {
    return ISelectable::GameObjectAssignableField(val, label);;
}