#include "TossEditor.h"
#include "Window.h"
#include "ProjectSettings.h"
#include "TossPlayerSettings.h"
#include "GraphicsEngine.h"
#include "EditorPlayer.h"
#include "ISelectable.h"
#include "FileWatcher.h"
#include "Camera.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <glfw3.h>

#include "imgui_internal.h"


TossEditor::TossEditor()
{
    editorPreferences.LoadFromFile("EditorPreferences.json");

    auto& tossEngine = TossEngine::GetInstance();
    tossEngine.Init();
    tossEngine.TryCreateWindow(this, editorPreferences.windowSize, "TossEditor", editorPreferences.maximized);
    tossEngine.LoadScripts();

    m_projectSettings = std::make_unique<ProjectSettings>();
    m_projectSettings->LoadFromFile("ProjectSettings.json");

    Physics::GetInstance().SetGravity(m_projectSettings->gravity);
    Physics::GetInstance().SetDebug(true);
    Physics::GetInstance().LoadPrefabWorld();
    AudioEngine::GetInstance().Init();

    ResourceManager& resourceManager = ResourceManager::GetInstance();
    tossEngine.StartCoroutine(resourceManager.loadResourceDesc("Resources/Resources.json"));
    tossEngine.StartCoroutine(resourceManager.createResourcesFromDescs());


    m_playerSettings = std::make_unique<TossPlayerSettings>();
    m_playerSettings->LoadFromFile("PlayerSettings.json");

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.Init(m_projectSettings);
    graphicsEngine.setViewport(editorPreferences.windowSize);
    graphicsEngine.setDepthFunc(DepthType::Less);
    graphicsEngine.setBlendFunc(BlendType::SrcAlpha, BlendType::OneMinusSrcAlpha);
    graphicsEngine.setFaceCulling(CullType::BackFace);
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
    graphicsEngine.setMultiSampling(true);

    auto& inputManager = InputManager::GetInstance();
    inputManager.Init(m_projectSettings);
    inputManager.setScreenArea(editorPreferences.windowSize);

    GeometryBuffer::GetInstance().Init(editorPreferences.windowSize);


    m_sceneFrameBuffer = std::make_shared<Framebuffer>(tossEngine.GetWindow()->getInnerSize());
    m_gameViewFrameBuffer = std::make_shared<Framebuffer>(tossEngine.GetWindow()->getInnerSize());

    Time::TimeScale = 0.0f;
}

TossEditor::~TossEditor()
{
}

void TossEditor::run()
{
    auto& tossEngine = TossEngine::GetInstance();

    try
    {
        onCreate();
        onCreateLate();

        while (!tossEngine.GetWindow()->shouldClose())
        {
            if (canUpdateInternal)
            {
                onUpdateInternal();
            }
            //check again as it can change
            if (canUpdateInternal)
            {
                onLateUpdateInternal();
            }
            tossEngine.PollEvents();
        }

        onQuit();
    }
    catch (...)
    {
        tossEngine.CleanUp();
        if (scriptWatcherThread.joinable())
            scriptWatcherThread.join();

        throw; // Rethrow to main()
    }

    // Normal cleanup if no exceptions
    tossEngine.CleanUp();
    if (scriptWatcherThread.joinable())
        scriptWatcherThread.join();
}

void TossEditor::onCreate()
{
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    auto& tossEngine = TossEngine::GetInstance();

    string filePath = editorPreferences.lastKnownOpenScenePath;

    if (!filePath.empty()) // If a file was selected
    {
        auto scene = std::make_shared<Scene>(filePath);
        tossEngine.OpenScene(scene, false);
    }
    else
    {
        auto scene = std::make_shared<Scene>("Scenes/Scene.json");
        tossEngine.OpenScene(scene, false);
    }

    sourceWatcher = new FileWatcher("C++Scripts/");

    //scriptWatcherThread = std::thread(&TossEditor::LoadWatchAndCompileScripts, this);

    m_player = std::make_unique<EditorPlayer>(this);
    m_player->onCreate();
}

void TossEditor::onCreateLate()
{
}

void TossEditor::DeleteSelected()
{
    if (selectedSelectable != nullptr)
    {
        if (selectedSelectable->Delete(false))
        {
            selectedSelectable = nullptr;
        }
    }
}

void TossEditor::Exit()
{
    TossEngine::GetInstance().GetWindow()->close();
}

void TossEditor::Reload()
{
    requestDllReload.store(true);
}

void TossEditor::DuplicateSelected()
{
    if (selectedSelectable)
    {
        if (auto gameObject = dynamic_cast<GameObject*>(selectedSelectable))
        {
            json gameObjectJsonData = gameObject->serialize();
            string name = gameObject->name;
            TossEngine::GetInstance().getGameObjectManager()->createGameObject<GameObject>(name, gameObjectJsonData);
        }
    }
}

void TossEditor::onUpdateInternal()
{
    TossEngine& tossEngine = TossEngine::GetInstance();
    InputManager& inputManager = InputManager::GetInstance();
    GraphicsEngine& graphicsEngine = GraphicsEngine::GetInstance();
    ResourceManager& resourceManager = ResourceManager::GetInstance();

    FindSceneFiles();

    // delta time
    m_currentTime = TossEngine::GetTime();
    float deltaTimeInternal = m_currentTime - m_previousTime;
    Time::UpdateDeltaTime(deltaTimeInternal);
    m_previousTime = m_currentTime;


    // Accumulate time
    m_accumulatedTime += deltaTimeInternal;
    resourceManager.onUpdateInternal();
    inputManager.onUpdate();

    if (auto scene = TossEngine::GetInstance().getCurrentScene())
    {
        if (m_gameRunning)
        {
            while (m_accumulatedTime >= Time::FixedDeltaTime)
            {
                scene->onFixedUpdate();
                m_accumulatedTime -= Time::FixedDeltaTime;
            }

        }
        // player update
        m_player->Update(deltaTimeInternal);
        scene->onUpdateInternal();


        if (m_gameRunning)
        {
            scene->onUpdate();
            scene->onLateUpdate();

            Physics::GetInstance().Update();
        }


        Physics::GetInstance().UpdateInternal();
    }

    graphicsEngine.clear(glm::vec4(0, 0, 0, 1)); //clear the existing stuff first is a must
    graphicsEngine.createImGuiFrame();

    ImGui::SetCurrentContext(graphicsEngine.getImGuiContext());
    ImGuizmo::SetImGuiContext(graphicsEngine.getImGuiContext());
    
    float menuBarHeight = ImGui::GetFrameHeightWithSpacing();  // More accurate with spacing
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    ImVec2 dockPos = viewport->Pos;
    dockPos.y += menuBarHeight;  // Offset by the menu bar height
    
    ImVec2 dockSize = viewport->Size;
    dockSize.y -= menuBarHeight; // Reduce the height accordingly
    
    ImGui::SetNextWindowPos(dockPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(dockSize, ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);
   
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground;
   
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);
    
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    const ImGuiWindowClass* window_class = nullptr;

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));
    ImGui::End();
    ImGuiIO& imGuiIo = ImGui::GetIO();
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New", "CTRL+N"))
            {
                CreateSceneFileViaFileSystem();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Open", "CTRL+0"))
            {
                OpenSceneViaFileSystem();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Save", "CTRL+S"))
            {
                Save();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Reload", "CTRL+R"))
            {
                Reload();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "CTRL+E"))
            {
                Exit();
            }
            ImGui::EndMenu();
        }
        ImGui::SameLine(0, 200);
        if (ImGui::Button("Play"))
        {
            if (!m_gameRunning)
            {
                Save();

                if (auto scene = TossEngine::GetInstance().getCurrentScene())
                {
                    Time::TimeScale = 1.0f;
                    m_gameRunning = true;
                    scene->onStart();
                    scene->onLateStart();
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop"))
        {
            //clean up
            if (m_gameRunning)
            {
                m_gameRunning = false;
                Time::TimeScale = 0.0f;

                string filePath = editorPreferences.lastKnownOpenScenePath;

                if (!filePath.empty()) // If a file was selected
                {
                    auto scene = std::make_shared<Scene>(filePath);
                    TossEngine::GetInstance().OpenScene(scene, false);
                }
                if(selectedSelectable != nullptr) selectedSelectable->OnDeSelect();
                
                selectedSelectable = nullptr;
            }
        }
        ImGui::EndMainMenuBar();
    }

    Debug::DrawConsole();

    ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {
        auto scene = TossEngine::GetInstance().getCurrentScene();
        // Get the available size for the Scene window.
        ImVec2 availSize = ImGui::GetContentRegionAvail();

        // Convert to your Vector2 type (assuming Vector2 takes width and height).
        Vector2 newSize(availSize.x, availSize.y);
        m_gameViewFrameBuffer->onResize(newSize);
        // Resize the scene's framebuffer to match the current window size.
        if (scene)
        {
            scene->onResize(newSize);
            scene->onGraphicsUpdate(nullptr, m_gameViewFrameBuffer);
        }


        // Get the updated texture after rendering.
        ImTextureID gameViewTexture = (ImTextureID)m_gameViewFrameBuffer->RenderTexture->getId();

        // Display the rendered scene scaled to the available region.
        ImGui::Image(gameViewTexture, availSize,
            ImVec2{ 0.f, 1.f },  // UV0
            ImVec2{ 1.f, 0.f }   // UV1 (flipped vertically if needed)
        );
    }
    ImGui::End();
    
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {
        // Get the available size for the Scene window.
        ImVec2 availSize = ImGui::GetContentRegionAvail();

        if (auto scene = TossEngine::GetInstance().getCurrentScene())
        {
            Vector2 newSize(availSize.x, availSize.y);
            Camera* camera = m_player->getCamera();
            // Resize the scene's framebuffer to match the current window size.
            camera->setScreenArea(newSize);
            scene->onResize(newSize);
            m_sceneFrameBuffer->onResize(newSize);

            Mat4 cameraView;
            m_player->getCamera()->getViewMatrix(cameraView);

            Mat4 projectionMat;
            m_player->getCamera()->getProjectionMatrix(projectionMat);

            // Now update the scene rendering with the current camera.
            scene->onGraphicsUpdate(camera, m_sceneFrameBuffer);

            // Get the updated texture after rendering.
            ImTextureID sceneViewTexture = (ImTextureID)m_sceneFrameBuffer->RenderTexture->getId();

            // Display the rendered scene scaled to the available region.
            ImGui::Image(sceneViewTexture, availSize, ImVec2{ 0.f, 1.f }, ImVec2{ 1.f, 0.f });
            if (selectedSelectable)
            {
                if (auto gameObject = dynamic_cast<GameObject*>(selectedSelectable))
                {
                    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
                    ImVec2 imagePos = ImGui::GetItemRectMin();
                    ImVec2 imageMaxPos = ImGui::GetItemRectMax();
                    ImVec2 imageSize = ImGui::GetItemRectSize();
                    ImGuizmo::SetRect(imagePos.x, imagePos.y, imageSize.x, imageSize.y);
                    //Debug::Log("Before Manipulate: Cursor Pos: (%f, %f)", pos.x, pos.y);

                    Mat4 transformMat = gameObject->m_transform.GetMatrix();

                    ImGui::PushClipRect(imagePos, imageMaxPos, true);

                    ImGuizmo::Manipulate(glm::value_ptr(cameraView.value), glm::value_ptr(projectionMat.value), ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::LOCAL, glm::value_ptr(transformMat.value));

                    ImGui::PopClipRect();
                    if (ImGuizmo::IsUsingAny())
                    {
                        Debug::Log("using any");
                        gameObject->m_transform.SetMatrix(transformMat);
                    }
                    if (ImGuizmo::IsOver())
                    {
                        //Debug::Log("over");
                    }
                    if (ImGuizmo::IsUsing())
                    {
                        Debug::Log("using");
                        gameObject->m_transform.SetMatrix(transformMat);
                    }
                }
            }
        }
    }
    ImGui::End();

    if (ImGui::Begin("SettingsAndBuild"))
    {
        ImGui::Text("Graphics Options");
        // Rendering Path selection
        static const char* items[]{ "Deferred Rendering", "Forward" };
        static int Selecteditem = (int)m_projectSettings->renderingPath;
        if (ImGui::Combo("Rendering Path", &Selecteditem, items, IM_ARRAYSIZE(items)))
        {
            RenderingPath selectedPath = static_cast<RenderingPath>(Selecteditem);
            Debug::Log("Rendering Path changed to option: " + ToString(selectedPath));
            graphicsEngine.setRenderingPath(selectedPath);
            m_projectSettings->renderingPath = selectedPath;
            m_playerSettings->renderingPath = selectedPath;
        }
        
        ImGui::Separator();
        ImGui::Text("Physics Settings");

        if (ImGui::DragFloat3("Gravity", m_projectSettings->gravity.Data(), 0.1f))
        {
            Physics::GetInstance().SetGravity(m_projectSettings->gravity);
        }

        ImGui::Separator();

        // --- Layer Manager Settings ---
        if (ImGui::CollapsingHeader("Layer Manager Settings"))
        {
            // Retrieve the singleton instance.
            LayerManager& layerManager = LayerManager::GetInstance();
            const auto& layers = layerManager.GetLayers();

            ImGui::Text("Current Layers:");

            // Create a vector copy of the keys to iterate safely.
            std::vector<std::string> layerNames;
            for (const auto& pair : layers)
            {
                layerNames.push_back(pair.first);
            }

            // Iterate over the copied list of layer names.
            for (const auto& name : layerNames)
            {
                ImGui::PushID(name.c_str());
                // Display the layer name and its bit value.
                ImGui::Text("%s : 0x%04X", name.c_str(), layers.at(name));

                bool deleteClicked = false;
                if (name != "Default")
                {
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Delete"))
                    {
                        deleteClicked = true;
                    }
                }
                ImGui::PopID();

                if (deleteClicked)
                {
                    // Remove the layer from the actual container.
                    if (layerManager.RemoveLayer(name))
                    {
                        Debug::Log("Layer deleted: " + name);
                    }
                    else
                    {
                        Debug::LogWarning("Failed to delete layer: " + name);
                    }
                    // Break out of the loop after a deletion to avoid further iteration over an outdated list.
                    break;
                }
            }

            // Input for a new layer name.
            static char newLayerName[128] = "";
            ImGui::InputText("New Layer Name", newLayerName, IM_ARRAYSIZE(newLayerName));

            // Button to add the new layer.
            if (ImGui::Button("Add Layer"))
            {
                if (strlen(newLayerName) > 0)
                {
                    try {
                        layerManager.GetLayer(newLayerName);
                        Debug::Log("Added new layer: " + std::string(newLayerName));
                        // Clear the text after adding.
                        newLayerName[0] = '\0';
                    }
                    catch (const std::exception& e)
                    {
                        Debug::LogError("Error adding layer: " + std::string(e.what()));
                    }
                }
            }
        }
        ImGui::Separator();

        ImGui::Text("Player Settings");

        ImGui::Text("All Scene File Paths:");

        // Ensure we have a selection flag for each scene file.
        static std::vector<bool> selected;
        if (selected.size() != allSceneFilePaths.size())
        {
            // Resize the selection flags vector.
            selected.resize(allSceneFilePaths.size(), false);

            // Set each flag to true if the file is already in selectedSceneFilePaths.
            for (size_t i = 0; i < allSceneFilePaths.size(); i++)
            {
                if (std::find(m_playerSettings->selectedSceneFilePaths.begin(),
                    m_playerSettings->selectedSceneFilePaths.end(),
                    allSceneFilePaths[i]) != m_playerSettings->selectedSceneFilePaths.end())
                {
                    selected[i] = true;
                }
            }
        }


        // Iterate over all scene file paths and display a checkbox for each.
        for (size_t i = 0; i < allSceneFilePaths.size(); i++)
        {
            std::string label = allSceneFilePaths[i];
            bool isSelected = selected[i];

            if (ImGui::Checkbox(label.c_str(), &isSelected))
            {
                selected[i] = isSelected;

                if (isSelected)
                {
                    // Add the file if not already selected.
                    if (std::find(m_playerSettings->selectedSceneFilePaths.begin(),
                        m_playerSettings->selectedSceneFilePaths.end(), label)
                        == m_playerSettings->selectedSceneFilePaths.end())
                    {
                        m_playerSettings->selectedSceneFilePaths.push_back(label);
                    }
                }
                else
                {
                    auto& vec = m_playerSettings->selectedSceneFilePaths;
                    vec.erase(std::remove(vec.begin(), vec.end(), label), vec.end());
                }
            }
        }

        ImGui::Text("First Scene To Open:");
        if (!m_playerSettings->selectedSceneFilePaths.empty())
        {
            // Static index to track the selected scene. Defaults to 0 (first option).
            static int selectedFirstSceneIndex = 0;

            // Build an array of C-string pointers from the vector of scene file paths.
            std::vector<const char*> sceneNames;
            for (const auto& scenePath : m_playerSettings->selectedSceneFilePaths)
            {
                sceneNames.push_back(scenePath.c_str());
            }

            // Display the combo box. If the user selects a different option, update the first scene.
            if (ImGui::Combo("##FirstSceneCombo", &selectedFirstSceneIndex, sceneNames.data(), sceneNames.size()))
            {
                m_playerSettings->firstSceneToOpen = m_playerSettings->selectedSceneFilePaths[selectedFirstSceneIndex];
            }
        }
        else
        {
            ImGui::Text("No scene files available.");
        }

        // Option to change the window size (assumed to have x and y components)
        int windowSize[2] = { static_cast<int>(m_playerSettings->windowSize.x), static_cast<int>(m_playerSettings->windowSize.y) };
        if (ImGui::InputInt2("Game Resolution", windowSize))
        {
            m_playerSettings->windowSize.x = static_cast<float>(windowSize[0]);
            m_playerSettings->windowSize.y = static_cast<float>(windowSize[1]);
        }


        // --- Build Button ---
        if (ImGui::Button("Build Project"))
        {
            requestBuild.store(true);
        }
    }
    ImGui::End();


    if (ImGui::Begin("Inspector"))
    {
        if (selectedSelectable)
        {
            ImGui::PushID(selectedSelectable);
            selectedSelectable->OnInspectorGUI();
            ImGui::PopID();
        }
        else
        {
            ImGui::Text("Nothing selected.");
        }
    }
    ImGui::End();

    if (ImGui::Begin("Hierarchy")) {
        auto scene = TossEngine::GetInstance().getCurrentScene();
        if (ImGui::Button("Add Empty GameObject") && scene) {
            scene->getObjectManager()->createGameObject<GameObject>();
        }

        // Display root game objects.
        if (scene) {
            for (const auto& pair : scene->getObjectManager()->m_gameObjects) {
                if (!pair.second) continue;

                // Only show root game objects.
                if (pair.second->m_transform.parent == nullptr) {
                    ShowGameObjectNode(pair.second);
                }
            }
        }

        // Add a drop target over the entire hierarchy window.
        ImVec2 windowSize = ImGui::GetContentRegionAvail();
        ImGui::InvisibleButton("HierarchyDropArea", windowSize);
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_RESOURCE")) {
                // Retrieve the raw pointer from the payload
                Resource* droppedResource = nullptr;
                memcpy(&droppedResource, payload->Data, payload->DataSize);

                if (droppedResource != nullptr) {
                    if (ResourcePtr resourcePtr = resourceManager.GetResourceByUniqueID(droppedResource->getUniqueID())) {
                        // Try to dynamically cast it to a Prefab
                        if (PrefabPtr prefab = std::dynamic_pointer_cast<Prefab>(resourcePtr)) {
                            // Create a new GameObject from the prefab data.
                            GameObject* newObj = scene->getObjectManager()->Instantiate(prefab, nullptr, Vector3(0.0f), Quaternion(), false);
                            Debug::Log("Created GameObject from prefab: " + prefab->getUniqueID());
                        }
                        else {
                            Debug::LogWarning("Dropped resource is not a prefab.");
                        }
                    }
                }
            }

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_GAMEOBJECT")) {
                GameObject* droppedGameObject = *static_cast<GameObject**>(payload->Data);
                if (droppedGameObject && droppedGameObject->m_transform.parent != nullptr) {
                    droppedGameObject->m_transform.SetParent(nullptr);
                }
            }
            ImGui::EndDragDropTarget();
        }
    }
    ImGui::End();

    if (renamingGameObject != nullptr)
    {
        ImGui::OpenPopup("Rename GameObject");
    }
    // Place the modal popup outside of any window blocks
    if (ImGui::BeginPopupModal("Rename GameObject", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("New name", renameBuffer, sizeof(renameBuffer));
        if (ImGui::Button("OK"))
        {
            if (renamingGameObject)
            {
                renamingGameObject->name = renameBuffer;
            }
            renamingGameObject = nullptr;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            renamingGameObject = nullptr;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::Begin("Assets")) {
        if (ImGui::Button("Create Resource")) {
            ImGui::OpenPopup("CreateResourcePopup");
        }

        // Resource creation popup
        if (ImGui::BeginPopup("CreateResourcePopup")) {
            if (ImGui::MenuItem("Texture2D")) {
                std::string textureFilePath = TossEngine::GetInstance().openFileDialog("*.png;*.jpg;*.jpeg;*.bmp");

                if (!textureFilePath.empty()) {
                    fs::path selectedPath = textureFilePath;
                    fs::path projectRoot = getProjectRoot();
                    fs::path relativePath;

                    if (fs::equivalent(selectedPath.root_name(), projectRoot.root_name()) &&
                        selectedPath.string().find(projectRoot.string()) == 0)
                    {
                        relativePath = fs::relative(selectedPath, projectRoot);
                        Debug::Log(relativePath.string());
                        resourceManager.createTexture2DFromFile(relativePath.string(), relativePath.string());//change this soon
                    }
                    else {
                        Debug::LogWarning("Selected texture must be inside the project folder.");
                    }
                }
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Mesh")) {
                meshFilepath = TossEngine::GetInstance().openFileDialog("*.obj");

                if (!meshFilepath.empty()) {
                    openMeshPopupNextFrame = true;
                }
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Prefab")) {
                openPrefabPopupNextFrame = true;
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Material")) {
                openMaterialPopupNextFrame = true;
                ImGui::CloseCurrentPopup();
            }
            
            if (ImGui::MenuItem("Cubemap Texture")) {
                openCubeMapPopupNextFrame = true;
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Shader")) {
                shaderVertPath = TossEngine::GetInstance().openFileDialog("*.vert");
                if (!shaderVertPath.empty()) {
                    shaderFragPath = TossEngine::GetInstance().openFileDialog("*.frag");
                    if (!shaderFragPath.empty()) {
                        openShaderPopupNextFrame = true; // Trigger name popup
                    }
                }
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Physics Material")) {
                openPhysicsMaterialNextFrame = true;
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Sound")) {
                soundFilePath = TossEngine::GetInstance().openFileDialog("*.ogg");
                if (!soundFilePath.empty()) {
                    openSoundCreationNextFrame = true;
                }
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        std::map<std::string, ResourcePtr>& resources = resourceManager.GetAllResources();
        std::vector<std::pair<std::string, ResourcePtr>> sortedResources(resources.begin(), resources.end());

        std::sort(sortedResources.begin(), sortedResources.end(),
            [](const auto& a, const auto& b) {
                return a.first < b.first;
            });

        for (auto& [uniqueID, resource] : sortedResources) {
            if (uniqueID.empty()) continue;

            bool isSelected = (resource == resourceManager.GetSelectedResource());

            std::string resourceLabel = uniqueID + " [" + getClassName(typeid(*resource)) + "]";

            if (ImGui::Selectable(resourceLabel.c_str(), isSelected)) {
                if (selectedSelectable != nullptr)
                    selectedSelectable->OnDeSelect();
                selectedSelectable = resource.get();
                selectedSelectable->OnSelect();
                resourceManager.SetSelectedResource(resource);
            }

            if (ImGui::BeginDragDropSource()) {
                Resource* resourceRaw = resource.get();
                ImGui::SetDragDropPayload("DND_RESOURCE", &resourceRaw, sizeof(resourceRaw));
                ImGui::Text("Drag '%s'", uniqueID.c_str());
                ImGui::EndDragDropSource();
            }

            std::string popupID = "item_context_" + uniqueID;
            if (ImGui::BeginPopupContextItem(popupID.c_str())) {
                if (ImGui::MenuItem("Rename")) {
                    resourceBeingRenamed = resource;
                    strncpy_s(renameBuffer, sizeof(renameBuffer), uniqueID.c_str(), _TRUNCATE);
                }
                if (ImGui::MenuItem("Delete")) {
                    resourceManager.DeleteResource(uniqueID);
                    if (isSelected) {
                        resourceManager.SetSelectedResource(nullptr);
                        selectedSelectable = nullptr;
                    }
                    ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                    break;
                }
                ImGui::EndPopup();
            }
        }


        // TODO: Create a function for drag and drop

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
        ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
        float scrollY = ImGui::GetScrollY();
        ImVec2 dropMin = ImVec2(windowPos.x + contentMin.x,
            windowPos.y + contentMin.y + scrollY);
        ImVec2 dropMax = ImVec2(windowPos.x + contentMax.x,
            windowPos.y + contentMax.y + scrollY);
        ImRect dropRect(dropMin, dropMax);
        if (ImGui::BeginDragDropTargetCustom(dropRect, ImGui::GetID("AssetsDropArea")))
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_GAMEOBJECT"))
            {
                // Retrieve the dropped GameObject.
                GameObject* droppedObject = *static_cast<GameObject**>(payload->Data);

                // Serialize the GameObject to JSON.
                json jsonData;
                Prefab::recurseSerialize(droppedObject, jsonData);

                // Create a prefab using the GameObject's name and JSON data.
                resourceManager.createPrefab(droppedObject->name, jsonData);

                Debug::Log("Created prefab '%s' from dragged GameObject", droppedObject->name.c_str());
            }
            ImGui::EndDragDropTarget();
        }
    }
    ImGui::End();

    // In the same frame, open the popups if flagged
    if (openMaterialPopupNextFrame) {
        ImGui::OpenPopup("Select Shader");
        openMaterialPopupNextFrame = false;
    }
    static char materialNameBuffer[64] = ""; // buffer for entering the material name

    if (ImGui::BeginPopupModal("Select Shader", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        std::string previewLabel = (shader) ? shader->getUniqueID() : "None";

        if (ImGui::BeginCombo("Shader", previewLabel.c_str()))
        {
            bool noneSelected = (shader == nullptr);
            if (ImGui::Selectable("None", noneSelected))
            {
                shader = nullptr;
                resourceManager.SetSelectedResource(nullptr);
            }

            auto& resources = resourceManager.GetAllResources();
            for (const auto& [id, resource] : resources)
            {
                auto casted = std::dynamic_pointer_cast<Shader>(resource);
                if (casted)
                {
                    bool isSelected = (shader && shader->getUniqueID() == id);
                    if (ImGui::Selectable(id.c_str(), isSelected))
                    {
                        shader = casted;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Spacing();
        ImGui::InputText("Material Name", materialNameBuffer, IM_ARRAYSIZE(materialNameBuffer));

        // Only enable the Create button if valid inputs are present
        bool canCreate = shader && strlen(materialNameBuffer) > 0;
        if (!canCreate)
            ImGui::BeginDisabled();

        if (ImGui::Button("Create Material"))
        {
            std::string name = materialNameBuffer;
            MaterialDesc materialDesc;
            materialDesc.shaderId = shader->getUniqueID();
            if (resourceManager.createMaterial(materialDesc, name))
            {
                Debug::Log("Created material: " + name + " using shader: " + shader->getUniqueID());
                ImGui::CloseCurrentPopup();
                materialNameBuffer[0] = '\0'; // Clear input field after creation
            }
            else
            {
                Debug::LogError("Failed to create material: " + name);
            }
        }

        if (!canCreate)
            ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
            materialNameBuffer[0] = '\0';
        }

        ImGui::EndPopup();
    }

    static char cubemapNameBuffer[64] = "NewCubemap";
    static bool filesSelected = false;  // To track if the files are selected
    static std::vector<std::string> cubemapFilePaths;  // Store file paths for the cubemap faces

    if (openCubeMapPopupNextFrame)
    {
        ImGui::OpenPopup("Create Cubemap Texture");
        openCubeMapPopupNextFrame = false;
    }

    if (ImGui::BeginPopupModal("Create Cubemap Texture", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("Cubemap Name", cubemapNameBuffer, IM_ARRAYSIZE(cubemapNameBuffer));

        // Face names for cubemap in typical order: +X, -X, +Y, -Y, +Z, -Z
        static const char* faceNames[6] = {
            "Right (+X)",
            "Left (-X)",
            "Top (+Y)",
            "Bottom (-Y)",
            "Back (-Z)",
            "Front (+Z)"
        };

        if (!filesSelected)
        {
            for (int i = cubemapFilePaths.size(); i < 6; ++i)
            {
                std::string label = std::string("Select ") + faceNames[i];
                ImGui::Text("%s", label.c_str());

                if (ImGui::Button(("Browse##" + std::to_string(i)).c_str()))
                {
                    std::string filePath = TossEngine::GetInstance().openFileDialog("*.png;*.jpg;");
                    if (!filePath.empty())
                    {
                        std::filesystem::path root = getProjectRoot();
                        std::filesystem::path relativePath = std::filesystem::relative(filePath, root);
                        cubemapFilePaths.push_back(relativePath.string());
                    }
                    break; // Only allow selecting one face per frame
                }
                break;
            }

            if (cubemapFilePaths.size() == 6)
            {
                filesSelected = true;
            }
        }

        ImGui::Separator();

        for (int i = 0; i < cubemapFilePaths.size(); ++i)
        {
            ImGui::Text("%s: %s", faceNames[i], cubemapFilePaths[i].c_str());
        }

        if (filesSelected && ImGui::Button("Create Cubemap"))
        {
            std::string cubemapName = cubemapNameBuffer; // Convert buffer to std::string
            ResourceManager::GetInstance().createCubeMapTextureFromFile(cubemapFilePaths, cubemapName);

            // Clear file paths and name buffer
            cubemapFilePaths.clear();
            std::fill(std::begin(cubemapNameBuffer), std::end(cubemapNameBuffer), 0);
            filesSelected = false;
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (openPrefabPopupNextFrame) {
        ImGui::OpenPopup("Enter Prefab ID");
        openPrefabPopupNextFrame = false;
    }

    if (ImGui::BeginPopupModal("Enter Prefab ID", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) 
    {
        ImGui::InputText("Prefab ID", iDBuffer, sizeof(iDBuffer));

        if (ImGui::Button("Create")) {
            if (strlen(iDBuffer) > 0) {
                std::string prefabID = iDBuffer;

                resourceManager.createPrefab(prefabID);

                Debug::Log("Created prefab: " + prefabID);

                // Reset state
                iDBuffer[0] = '\0';

                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            meshFilepath.clear();
            iDBuffer[0] = '\0';

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (openMeshPopupNextFrame) {
        ImGui::OpenPopup("Enter Mesh ID");
        openMeshPopupNextFrame = false;
    }

    // Show the popup to name the shader
    if (ImGui::BeginPopupModal("Enter Mesh ID", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Mesh ID", iDBuffer, sizeof(iDBuffer));

        if (ImGui::Button("Create")) {
            if (strlen(iDBuffer) > 0) {
                std::string meshId = iDBuffer;
                std::filesystem::path root = getProjectRoot();
                std::filesystem::path relativeMeshPath = std::filesystem::relative(meshFilepath, root);

                MeshDesc desc;
                desc.filePath = relativeMeshPath.string();

                resourceManager.createMesh(desc, meshId);

                Debug::Log("Created mesh: " + meshId);

                // Reset state
                meshFilepath.clear();
                iDBuffer[0] = '\0';

                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            meshFilepath.clear();
            iDBuffer[0] = '\0';

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // In the same frame, open the popup if flagged
    if (openShaderPopupNextFrame) {
        ImGui::OpenPopup("Enter Shader ID");
        openShaderPopupNextFrame = false;
    }
    
    if (ImGui::BeginPopupModal("Enter Shader ID", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Shader ID", shaderIDBuffer, sizeof(shaderIDBuffer));

        if (ImGui::Button("Create")) {
            if (strlen(shaderIDBuffer) > 0) {
                std::string shaderID = shaderIDBuffer;
                std::filesystem::path root = getProjectRoot();
                std::filesystem::path vertRel = std::filesystem::relative(shaderVertPath, root);
                std::filesystem::path fragRel = std::filesystem::relative(shaderFragPath, root);

                ShaderDesc desc;
                desc.vertexShaderFilePath = vertRel.string();
                desc.fragmentShaderFilePath = fragRel.string();

                resourceManager.createShader(desc, shaderID);

                Debug::Log("Created shader: " + shaderID);

                // Reset state
                shaderVertPath.clear();
                shaderFragPath.clear();
                shaderIDBuffer[0] = '\0';

                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            shaderVertPath.clear();
            shaderFragPath.clear();
            shaderIDBuffer[0] = '\0';

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }


    if (openSoundCreationNextFrame) {
        ImGui::OpenPopup("Enter Sound ID");
        openSoundCreationNextFrame = false;
    }

    // Show the popup to name the shader
    if (ImGui::BeginPopupModal("Enter Sound ID", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Sound ID", IDBuffer, sizeof(IDBuffer));

        if (ImGui::Button("Create")) {
            if (strlen(IDBuffer) > 0) {

                std::filesystem::path root = getProjectRoot();
                std::filesystem::path soundFilePathRel = std::filesystem::relative(soundFilePath, root);
                std::string soundId = IDBuffer;

                SoundDesc desc;
                desc.filepath = soundFilePathRel.string();
                resourceManager.createSound(desc, soundId);
                Debug::Log("Created sound: " + soundId);

                // Reset state
                IDBuffer[0] = '\0';

                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            IDBuffer[0] = '\0';

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (openPhysicsMaterialNextFrame) {
        ImGui::OpenPopup("Enter Physics Material ID");
        openPhysicsMaterialNextFrame = false;
    }

    // Show the popup to name the shader
    if (ImGui::BeginPopupModal("Enter Physics Material ID", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Physics Material ID", IDBuffer, sizeof(IDBuffer));

        if (ImGui::Button("Create")) {
            if (strlen(IDBuffer) > 0) {
                std::string physicsMaterialID = IDBuffer;

                PhysicsMaterialDesc desc;
                resourceManager.createPhysicsMaterial(desc, physicsMaterialID);
                Debug::Log("Created physics material: " + physicsMaterialID);

                // Reset state
                IDBuffer[0] = '\0';

                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            IDBuffer[0] = '\0';

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (resourceBeingRenamed != nullptr)
    {
        ImGui::OpenPopup("Rename Resource");
    }
    if (ImGui::BeginPopupModal("Rename Resource", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("New ID", renameBuffer, sizeof(renameBuffer));

        if (ImGui::Button("OK"))
        {
            if (resourceBeingRenamed && strlen(renameBuffer) > 0)
            {
                std::string oldID = resourceBeingRenamed->getUniqueID();
                std::string newID = renameBuffer;

                // Rename the resource
                resourceManager.RenameResource(resourceBeingRenamed, newID);
            }

            resourceBeingRenamed = nullptr;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            resourceBeingRenamed = nullptr;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    graphicsEngine.renderImGuiFrame();
    

    if (imGuiIo.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    // Render to window
    tossEngine.GetWindow()->present();


}

void TossEditor::onLateUpdateInternal()
{
    InputManager::GetInstance().onLateUpdate();

    if (requestDllReload.load())
    {
        requestDllReload.store(false);
        Debug::Log("Reloading DLL safely at end of frame...");
        PerformSafeDllReload();
    }
    if (requestBuild.load())
    {
        requestBuild.store(false);
        Debug::Log("Building player safely at end of frame...");
        PerformSafeBuild();
    }
}

void TossEditor::FindSceneFiles()
{
    // Clear the current list to update it with any changes.
    allSceneFilePaths.clear();

    // Check if the "Scenes" directory exists.
    const std::filesystem::path scenesDir("Scenes");
    if (std::filesystem::exists(scenesDir) && std::filesystem::is_directory(scenesDir))
    {
        // Iterate over files in the directory.
        for (const auto& entry : std::filesystem::directory_iterator(scenesDir))
        {
            // Ensure it is a regular file and has a .json extension.
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                allSceneFilePaths.push_back(entry.path().string());
            }
        }
    }
}

void TossEditor::onQuit()
{
    m_editorRunning.store(false);

    if (auto scene = TossEngine::GetInstance().getCurrentScene())
    {
        scene->onQuit();
    }
    ResourceManager& resourceManager = ResourceManager::GetInstance();
    TossEngine::GetInstance().StartCoroutine(resourceManager.saveResourcesDescs("Resources/Resources.json"));
    resourceManager.CleanUp();
    Physics::GetInstance().UnLoadPrefabWorld();
    editorPreferences.SaveToFile("EditorPreferences.json");
}

void TossEditor::ShowGameObjectNode(GameObject* gameObject)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
    if (gameObject->m_transform.children.empty())
        flags |= ImGuiTreeNodeFlags_Leaf;
    if (gameObject == selectedSelectable)
        flags |= ImGuiTreeNodeFlags_Selected;

    // Create the tree node, which draws the arrow and the label.
    bool open = ImGui::TreeNodeEx(gameObject->name.c_str(), flags);

    // Get the bounding rectangle for this item to determine where the arrow is drawn.
    ImVec2 itemMin = ImGui::GetItemRectMin();
    // Typically, the arrow is about one frame height wide.
    float arrowWidth = ImGui::GetFrameHeight();

    // If the tree node was clicked...
    if (ImGui::IsItemClicked())
    {
        // Check if the click was outside of the arrow region.
        // That is, if the mouse's x position is farther right than the arrow's width,
        // then treat the click as a selection of the game object.
        if (ImGui::GetIO().MousePos.x > itemMin.x + arrowWidth)
        {
            // Deselect the previous selection if any.
            if (selectedSelectable != nullptr)
                selectedSelectable->OnDeSelect();
            selectedSelectable = gameObject;
            selectedSelectable->OnSelect();
        }
    }
    // Drag and drop source: allow dragging this game object.
    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("DND_GAMEOBJECT", &gameObject, sizeof(GameObject*));
        ImGui::Text("Dragging %s", gameObject->name.c_str());
        ImGui::EndDragDropSource();
    }

    // Drag and drop target: allow accepting a dropped game object.
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_GAMEOBJECT"))
        {
            GameObject* droppedGameObject = *static_cast<GameObject**>(payload->Data);
            if (droppedGameObject != gameObject)
            {
                droppedGameObject->m_transform.SetParent(&gameObject->m_transform, false);
            }
        }
        ImGui::EndDragDropTarget();
    }

    // Context menu (right-click) for renaming/deleting.
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Rename"))
        {
            renamingGameObject = gameObject;
            strncpy_s(renameBuffer, sizeof(renameBuffer), gameObject->name.c_str(), _TRUNCATE);
            Debug::Log("Rename selected for " + gameObject->name);
        }
        if (ImGui::MenuItem("Delete"))
        {
            gameObject->Delete();
            if (selectedSelectable == gameObject)
                selectedSelectable = nullptr;
        }
        ImGui::EndPopup();
    }

    // If the node is expanded, recursively show its children.
    if (open)
    {
        for (Transform* child : gameObject->m_transform.children)
        {
            ShowGameObjectNode(child->gameObject);
        }
        ImGui::TreePop();
    }
}


void TossEditor::LoadWatchAndCompileScripts()
{
    while (m_editorRunning.load())
    {
        try {
            if (sourceWatcher->hasChanged()) {
                Debug::Log("Scripts folder changed. Marking DLL for reload...");
                requestDllReload.store(true); // atomic flag
            }
        }
        catch (const std::exception& e) {
            Debug::LogWarning(std::string("[Watcher] Exception: ") + e.what());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void TossEditor::PerformSafeBuild()
{
    std::string msBuildPath = getMSBuildPath();
    std::string solutionPath = FindSolutionPath(); //this needs to change to proper name of the solution on the release build of the engine
    std::string config;

    #ifdef _DEBUG
        config = "Debug";
    #else
        config = "Release";
    #endif

    if (msBuildPath.empty()) {
        Debug::LogError("No buildpath (MSBuild.exe not found)", false);
        return;
    }

    if (solutionPath.empty()) {
        Debug::LogError("Could not locate sln | did not compile build", false);
        return;
    }

    TossEngine& tossEngine = TossEngine::GetInstance();
    tossEngine.UnLoadScripts();

    std::string command = std::string("cmd /C \"\"") + msBuildPath +
        "\" \"" + solutionPath +
        "\" /t:TossPlayer /p:Configuration=" + config + " /p:Platform=x64\"";

    int result = system(command.c_str());

    if (result == 0) {
        Debug::Log("TossPlayer Project compiled successfully.");
    }
    else {
        Debug::LogError("TossPlayer Project compilation failed.");
    }

    tossEngine.LoadScripts();
}

void TossEditor::PerformSafeDllReload()
{
    ResourceManager& resourceManager = ResourceManager::GetInstance();
    //Save();
    auto scene = TossEngine::GetInstance().getCurrentScene();
    resourceManager.saveResourcesDescs("Resources/Resources.json"); // save resources just in case of crash and to save prefabs
    selectedSelectable = nullptr; //because we are reloading the scene we can have to pointing to a old object
    canUpdateInternal = false;
    if (scene) scene->clean();
    resourceManager.DeletePrefabs();
    TossEngine::GetInstance().ReloadScripts();
    resourceManager.LoadPrefabs();
    if (scene) scene->reload(); // reloading scene for new script types or changes to scripts

    canUpdateInternal = true;
}

void TossEditor::onResize(Vector2 size)
{
    Resizable::onResize(size);
    editorPreferences.windowSize = size;
}

void TossEditor::onMaximize(int maximized)
{
    Resizable::onMaximize(maximized);
    editorPreferences.maximized = maximized;
}

void TossEditor::Undo()
{
}

void TossEditor::Save() const
{
    m_projectSettings->SaveToFile("ProjectSettings.json");
    m_playerSettings->SaveToFile("PlayerSettings.json");
    if (auto scene = TossEngine::GetInstance().getCurrentScene())
    {
        scene->Save();
    }
    LayerManager& layerManager = LayerManager::GetInstance();
    json data = layerManager.serialize(); 
    JsonUtility::SaveJsonFile("LayerManager.json", data);
}

void TossEditor::CreateSceneFileViaFileSystem()
{
}

void TossEditor::OpenSceneViaFileSystem()
{
    string sceneFilePath = TossEngine::GetInstance().openFileDialog("*.json");

    if (!sceneFilePath.empty()) {
        fs::path selectedPath = sceneFilePath;
        fs::path projectRoot = getProjectRoot();
        fs::path relativeSceneFilePath;

        if (fs::equivalent(selectedPath.root_name(), projectRoot.root_name()) &&
            selectedPath.string().find(projectRoot.string()) == 0)
        {
            relativeSceneFilePath = fs::relative(selectedPath, projectRoot);
            editorPreferences.lastKnownOpenScenePath = relativeSceneFilePath.string();
            auto scene = std::make_shared<Scene>(relativeSceneFilePath.string());
            TossEngine::GetInstance().OpenScene(scene, false);
        }
        else {
            Debug::LogWarning("Selected scene file must be inside the project folder.");
        }
    }
}
//void DrawSceneViewWindow(FrameBuffer* editorFB,
//    GameObject* editorCamera,
//    Scene* scene,
//    GameObject* selected,
//    float deltaTime)
//{
//    editorCamera->Update(deltaTime);
//    editorFB->Bind();
//    scene->Render(editorFB, editorCamera->GetComponent<Camera>());
//    editorFB->Unbind();
//
//    ImGui::Begin("Scene View");
//
//    ImVec2 sz((float)editorFB->GetWidth(), (float)editorFB->GetHeight());
//    ImGui::Image((ImTextureID)(intptr_t)editorFB->GetTextureID(), sz);
//
//    ImVec2 img_min = ImGui::GetItemRectMin();
//    ImVec2 img_max = ImGui::GetItemRectMax();
//    ImVec2 img_sz{ img_max.x - img_min.x, img_max.y - img_min.y };
//
//    ImGuizmo::BeginFrame();
//    ImGuizmo::Enable(selected != nullptr);
//    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
//    ImGuizmo::SetOrthographic(false);
//    ImGuizmo::SetRect(img_min.x, img_min.y, img_sz.x, img_sz.y);
//
//    Camera* cam = editorCamera->GetComponent<Camera>();
//    glm::mat4 view = cam->GetViewMatrix();
//    glm::mat4 proj = cam->GetProjectionMatrix();
//    proj[1][1] *= -1.0f;
//
//    if (selected)
//    {
//        ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
//
//        glm::mat4 model = selected->GetWorldMatrix();
//
//        ImGui::PushClipRect(img_min, img_max, true);
//
//        bool depth = glIsEnabled(GL_DEPTH_TEST);
//        if (depth) glDisable(GL_DEPTH_TEST);
//
//        ImGuizmo::Manipulate(
//            glm::value_ptr(view),
//            glm::value_ptr(proj),
//            op,
//            ImGuizmo::LOCAL,
//            glm::value_ptr(model)
//        );
//
//        if (depth) glEnable(GL_DEPTH_TEST);
//
//        ImGui::PopClipRect();
//
//        if (ImGuizmo::IsUsing()) {
//            glm::vec3 t, r, s;
//            ImGuizmo::DecomposeMatrixToComponents(
//                glm::value_ptr(model),
//                glm::value_ptr(t),
//                glm::value_ptr(r),
//                glm::value_ptr(s)
//            );
//            selected->transform.position = t;
//            selected->transform.rotation = glm::quat(glm::radians(r));
//            selected->transform.scale = s;
//        }
//    }
//
//    ImGui::End();
//}