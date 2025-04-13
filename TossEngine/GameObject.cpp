/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : GameObject.cpp
Description : GameObject class that represents an object for OpenGl with its own update and onCreate functions.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "GameObject.h"
#include "GameObjectManager.h"
#include "ComponentRegistry.h"
#include "Component.h"

GameObject::GameObject() : m_transform(this)
{
}

GameObject::GameObject(const GameObject& other) : m_transform(this)
{
    m_transform = other.m_transform;
    m_id = other.m_id;
    m_gameObjectManager = other.m_gameObjectManager;

    // Deep copy all components
    for (const auto& pair : other.m_components)
    {
        // Clone the component and add it to the new GameObject
        //Component* clonedComponent = pair.second->Clone();
        //if (clonedComponent)
        //{
        //    clonedComponent->setOwner(this); // Set the owner of the cloned component
        //    m_components.emplace(pair.first, clonedComponent); // Add the cloned component to the map
        //}
    }
}

GameObject::~GameObject()
{
    for (auto& pair : m_components) {
        pair.second->onDestroy();
    }
    for (auto& pair : m_components)
    {
        delete pair.second; // Free allocated memory
    }
    m_components.clear();
}

json GameObject::serialize() const
{
    json componentsJson = json::array();
    for (const auto& pair : m_components)
    {
        componentsJson.push_back(pair.second->serialize());
    }
    return {
        {"type", getClassName(typeid(*this))}, // Use typeid to get the class name
        {"id", m_id},
        {"name", name},
        {"layer", m_layer},
        {"tag", tag},
        { "transform", m_transform.serialize() },
        {"components", componentsJson}
    };
}

void GameObject::deserialize(const json& data)
{
    if (data.contains("name"))
    {
        name = data["name"];
    }
    if (data.contains("layer"))
    {
        m_layer = data["layer"];
    }
    if (data.contains("tag"))
    {
        tag = data["tag"];
    }
    if (data.contains("transform"))
    {
        m_transform.deserialize(data);
    }
    if (data.contains("transform"))
    {
        m_transform.deserialize(data["transform"]);
    }
    //m_transform.SetParent();

    if (data.contains("components"))
    {
        for (const auto& componentData : data["components"])
        {
            if (componentData.contains("type"))
            {
                addComponent(componentData["type"], componentData);
            }
            else
            {
                Debug::LogError("Error loading component on gameobject: " + name + ". As the component has no type");
            }
        }
    }
}

void GameObject::OnInspectorGUI()
{
    ImGui::Text("Selected Object: %s", name.c_str());

    // You can define the size of the buffer based on your needs
    strncpy_s(tagBuffer, tag.c_str(), sizeof(tagBuffer)); // Copy the existing tag into the buffer
    tagBuffer[sizeof(tagBuffer) - 1] = '\0';

    if (ImGui::InputText("Tag", tagBuffer, sizeof(tagBuffer)))
    {
        tag = std::string(tagBuffer);
    }
    
    // Retrieve available layers from the LayerManager.
    LayerManager& layerManager = LayerManager::GetInstance();
    const auto& availableLayers = layerManager.GetLayers();

    // Build a vector of all available layer names.
    std::vector<std::string> allLayerNames;
    for (const auto& pair : availableLayers) {
        allLayerNames.push_back(pair.first);
    }

    // If no layer is set, default to "Default".
    if (m_layer.empty()) {
        m_layer = "Default";
    }

    // Find the index of the currently selected layer.
    int selectedIndex = 0;
    for (int i = 0; i < allLayerNames.size(); i++) {
        if (allLayerNames[i] == m_layer) {
            selectedIndex = i;
            break;
        }
    }

    // Display the combo box using the currently selected layer as the preview.
    if (ImGui::BeginCombo("Layer", m_layer.c_str())) {
        for (int i = 0; i < allLayerNames.size(); i++) {
            bool isSelected = (selectedIndex == i);
            if (ImGui::Selectable(allLayerNames[i].c_str(), isSelected)) {
                m_layer = allLayerNames[i];
                selectedIndex = i;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();
    ImGui::Text("Transform:");
    ImGui::DragFloat3("Position", m_transform.localPosition.Data(), 0.1f);

    if (ImGui::DragFloat3("Rotation", eulerAngles.Data(), 0.1f))
    {
        // Convert the edited angles back to radians and update the quaternion
        m_transform.localRotation = Quaternion(eulerAngles.ToRadians());
    }
    else
    {
        eulerAngles = m_transform.localRotation.ToEulerAngles().ToDegrees();
    }

    ImGui::DragFloat3("Scale", m_transform.localScale.Data(), 0.1f);
    for (auto& pair : m_components)
    {
        ImGui::Separator();
        Component* comp = pair.second; // e.g., from a std::map
        // Highlight the selected component.
        ImGuiTreeNodeFlags flags = (selectedComponent == comp) ? ImGuiTreeNodeFlags_Selected : 0;
        bool open = ImGui::TreeNodeEx(comp->getName().c_str(), flags);
        if (ImGui::IsItemClicked())
        {
            // Select the component when clicked.
            selectedComponent = comp;
        }
        // Right-click context menu.
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Delete"))
            {
                removeComponent(comp);
                if (selectedComponent == comp)
                    selectedComponent = nullptr;
            }
            ImGui::EndPopup();
        }
        if (open)
        {
            if (comp)
            {

                comp->OnInspectorGUI();
            }
            ImGui::TreePop();
        }
    }
    ImGui::Separator();

    // "Add Component" button and popup remain unchanged.
    if (ImGui::Button("Add Component"))
    {
        ImGui::OpenPopup("Add Component Popup");
    }
    static char componentSearchBuffer[256] = "";
    if (ImGui::BeginPopup("Add Component Popup"))
    {
        ImGui::InputText("Search", componentSearchBuffer, sizeof(componentSearchBuffer));
        auto& registry = ComponentRegistry::GetInstance();
        std::vector<std::string> componentTypes = registry.getRegisteredComponentNames();
        for (const auto& compName : componentTypes)
        {
            if (componentSearchBuffer[0] == '\0' || std::strstr(compName.c_str(), componentSearchBuffer) != nullptr)
            {
                if (ImGui::Selectable(compName.c_str()))
                {
                    addComponent(compName);
                }
            }
        }
        ImGui::EndPopup();
    }
}

void GameObject::OnSelect()
{
    for (auto& pair : m_components)
    {
        pair.second->OnGameObjectSelected();
    }
}


void GameObject::OnDeSelect()
{
    for (auto& pair : m_components)
    {
        pair.second->OnGameObjectDeSelected();
    }
}

size_t GameObject::getId()
{
    return m_id;
}

LayerBit GameObject::getLayer()
{
    return LayerManager::GetInstance().GetLayer(m_layer);
}

void GameObject::setId(size_t id)
{
    m_id = id;
}

void GameObject::onCreate()
{
}

void GameObject::onLateCreate()
{
    for (auto& pair : m_components) {
        pair.second->onLateCreate();
    }
    m_finishedCreation = true;
}

void GameObject::onStart()
{
    for (auto& pair : m_components) {
        pair.second->onStart();
    }
}

void GameObject::onLateStart()
{
    for (auto& pair : m_components) {
        pair.second->onLateStart();
    }

    hasStarted = true;
}

void GameObject::onFixedUpdate(float fixedDeltaTime)
{
    for (auto& pair : m_components) {
        pair.second->onFixedUpdate(fixedDeltaTime);
    }
}

void GameObject::onUpdate()
{
    m_transform.UpdateWorldTransform();

    for (auto& pair : m_components) {
        pair.second->onUpdate();
    }
}

void GameObject::onUpdateInternal()
{
    if (Time::TimeScale != 0.0f) return;

    m_transform.UpdateWorldTransform();

    for (Component* component : componentsToDestroy)
    {
        // Compute the key using the dynamic type of the component.
        std::type_index key(typeid(*component));

        // Look for the component in the container.
        auto it = m_components.find(key);
        if (it != m_components.end() && it->second == component)
        {
            component->onDestroy();

            // Remove the component from the container.
            m_components.erase(it);

            // Delete the component (if you own it, which is the case here since addComponent used new).
            delete component;
        }
    }
    componentsToDestroy.clear();

    for (auto& pair : m_components) {
        pair.second->onUpdateInternal();
    }
}

void GameObject::onLateUpdate()
{
}

void GameObject::onDestroy()
{
    for (auto& pair : m_components) 
    {
        pair.second->onDestroy();
    }
}

void GameObject::CallOnCollisionEnterCallbacks(Collider* other) const
{
    for (auto& pair : m_components) {
        pair.second->onCollisionEnter(other);
    }
}

void GameObject::CallOnCollisionExitCallbacks(Collider* other) const
{
    for (auto& pair : m_components) {
        pair.second->onCollisionExit(other);
    }
}

void GameObject::CallOnTriggerEnterCallbacks(Collider* other) const
{
    Debug::Log("Enter");
    for (auto& pair : m_components) {
        pair.second->onTriggerEnter(other);
    }
}

void GameObject::CallOnTriggerExitCallbacks(Collider* other) const
{
    Debug::Log("Exit");
    for (auto& pair : m_components) {
        pair.second->onTriggerExit(other);
    }
}

Component* GameObject::addComponent(string componentType, const json& data)
{
    auto component = ComponentRegistry::GetInstance().createComponent(componentType);
    if (component)
    {
        component->setOwner(this);
        m_components.emplace(std::type_index(typeid(*component)), component);
        component->onCreate();
        if (data != nullptr)
        {
            component->deserialize(data);
        }
        if (m_finishedCreation)
        {
            component->onLateCreate();
        }

        if (hasStarted)
        {
            component->onStart();
            component->onLateStart();
        }

        return component;
    }
    else
    {
        Debug::LogError("Error adding component of type: " + componentType + " to gameobject: " + ToString(this->m_id), false);
    }
    return nullptr;
}

void GameObject::removeComponent(Component* component)
{
    if (component == nullptr)
        return;

    componentsToDestroy.push_back(component);
}

bool GameObject::tryDeleteSelectedComponent()
{
    if (selectedComponent != nullptr)
    {
        removeComponent(selectedComponent);
        selectedComponent = nullptr;
        return true;
    }
    return false;
}

void GameObject::setGameObjectManager(GameObjectManager* gameObjectManager)
{
    // Set the GameObjectManager managing this GameObject
    m_gameObjectManager = gameObjectManager;
}

bool GameObject::Delete(bool deleteSelf)
{
    // If there's a selected component and we're not deleting the game object itself,
    // delete only the component.
    if (selectedComponent != nullptr && !deleteSelf)
    {
        selectedComponent->Delete();
        selectedComponent = nullptr;
        return false;
    }
    else
    {
        // Make a copy of the children list since deleting children might modify the original vector.
        std::vector<Transform*> childrenCopy = m_transform.children;
        for (Transform* childTransform : childrenCopy)
        {
            if (childTransform && childTransform->gameObject)
            {
                // Recursively delete the child game object.
                childTransform->gameObject->Delete(true);
            }
        }
        // Remove this game object from the game object manager.
        m_gameObjectManager->removeGameObject(this);
        return true;
    }
}

GameObjectManager* GameObject::getGameObjectManager() const
{
    return m_gameObjectManager;
}

LightManager* GameObject::getLightManager() const
{
    return m_gameObjectManager->getScene()->getLightManager();
}
