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
        {"transform", {
            {"position", {m_transform.position.x, m_transform.position.y, m_transform.position.z}},
            {"rotation", {m_transform.rotation.x, m_transform.rotation.y, m_transform.rotation.z, m_transform.rotation.w}},
            {"scale", {m_transform.scale.x, m_transform.scale.y, m_transform.scale.z}},
            {"localPosition", {m_transform.localPosition.x, m_transform.localPosition.y, m_transform.localPosition.z}},
            {"localRotation", {m_transform.localRotation.x, m_transform.localRotation.y, m_transform.localRotation.z, m_transform.localRotation.w}},
            {"localScale", {m_transform.localScale.x, m_transform.localScale.y, m_transform.localScale.z}},
            {"parent", m_transform.parent ? m_transform.parent->gameObject->getId() : 0 }
        }},
        {"components", componentsJson}
    };
}

void GameObject::deserialize(const json& data) 
{
    if (data.contains("name"))
    {
        name = data["name"];
    }
    if (data.contains("transform"))
    {
        auto transformData = data["transform"];
        if (transformData.contains("id"))
        {
            m_id = (size_t)transformData["id"];
        }
        if (transformData.contains("position"))
        {
            auto pos = transformData["position"];
            m_transform.position = Vector3(pos[0], pos[1], pos[2]);
        }
        if (transformData.contains("rotation"))
        {
            auto rot = transformData["rotation"];
            m_transform.rotation = Quaternion(rot[3], rot[0], rot[1], rot[2]);
        }
        if (transformData.contains("scale"))
        {
            auto scl = transformData["scale"];
            m_transform.scale = Vector3(scl[0], scl[1], scl[2]);
        }
        if (transformData.contains("localPosition"))
        {
            auto pos = transformData["localPosition"];
            m_transform.localPosition = Vector3(pos[0], pos[1], pos[2]);
        }
        if (transformData.contains("localRotation"))
        {
            auto rot = transformData["localRotation"];
            m_transform.localRotation = Quaternion(rot[3], rot[0], rot[1], rot[2]);
        }
        if (transformData.contains("localScale"))
        {
            auto scl = transformData["localScale"];
            m_transform.localScale = Vector3(scl[0], scl[1], scl[2]);
        }
        if (transformData.contains("parent") && transformData["parent"] != 0)
        {
            size_t parentID = transformData["parent"].get<size_t>();
        
            auto& gameObjects = getGameObjectManager()->m_gameObjects;
            auto it = gameObjects.find(parentID);
            if (it != gameObjects.end())
            {
                // Set the parent transform safely
                m_transform.SetParent(&it->second->m_transform);
                Debug::Log("Parent Set");
            }
            else
            {
                // Handle the case where parentID is invalid/not found
                m_transform.SetParent(nullptr);
            }
        }
    }
    eulerAngles = m_transform.ToEulerAngles();
    m_transform.UpdateWorldTransform();
    //m_transform.SetParent();

    if (data.contains("components"))
    {
        for (const auto& componentData : data["components"])
        {
            std::string componentType = componentData["type"];
            addComponent(componentType, componentData);
        }
    }
}

void GameObject::OnInspectorGUI()
{
    ImGui::Text("Selected Object: %s", name.c_str());
    ImGui::Separator();
    ImGui::Text("Transform:");
    ImGui::DragFloat3("Position", glm::value_ptr(m_transform.position), 0.1f);
    // Convert from radians to degrees for display
    
    if (ImGui::DragFloat3("Rotation", glm::value_ptr(eulerAngles), 0.1f))
    {
        // Convert the edited angles back to radians and update the quaternion
        m_transform.rotation = glm::normalize(glm::quat(glm::radians(eulerAngles)));
    }

    ImGui::DragFloat3("Scale", glm::value_ptr(m_transform.scale), 0.1f);
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
            comp->OnInspectorGUI();
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

size_t GameObject::getId()
{
	return m_id;
}

void GameObject::setId(size_t id)
{
	m_id = id;
}

void GameObject::onCreate()
{
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
}

void GameObject::onFixedUpdate(float fixedDeltaTime)
{
    for (auto& pair : m_components) {
        pair.second->onFixedUpdate(fixedDeltaTime);
    }
}

void GameObject::onUpdate(float deltaTime)
{
    m_transform.UpdateWorldTransform();

    for (auto& pair : m_components) {
        pair.second->onUpdate(deltaTime);
    }
}

void GameObject::onUpdateInternal()
{
    for (Component* component : componentsToDestroy)
    {
        // Compute the key using the dynamic type of the component.
        std::type_index key(typeid(*component));

        // Look for the component in the container.
        auto it = m_components.find(key);
        if (it != m_components.end() && it->second == component)
        {
            // Optionally, call a cleanup method (e.g. onDestroy) if your component defines one.
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

void GameObject::onLateUpdate(float deltaTime)
{
}

Component* GameObject::addComponent(string componentType, const json& data)
{
    auto component = ComponentRegistry::GetInstance().createComponent(componentType);
    if (component)
    {
        component->setOwner(this);
        component->onCreate();
        if (data != nullptr)
        {
            component->deserialize(data);
        }
        m_components.emplace(std::type_index(typeid(*component)), component);
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
    if (selectedComponent != nullptr && !deleteSelf)
    {
        selectedComponent->Delete();
        selectedComponent = nullptr;
        return false;
    }
    else
    {
        m_gameObjectManager->removeGameObject(this);
        return true;
    }
}

GameObjectManager* GameObject::getGameObjectManager()
{
	return m_gameObjectManager;
}

LightManager* GameObject::getLightManager()
{
    return m_gameObjectManager->getScene()->getLightManager();
}
