#include "TossEditor.h"
#include "Window.h"
#include "ProjectSettings.h"
#include "TossPlayerSettings.h"
#include "GraphicsEngine.h"
#include "EditorPlayer.h"
#include "ISelectable.h"
#include "ScriptLoader.h"
#include "FileWatcher.h"
#include "Camera.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <glfw3.h>

#include "TestClass.h"
#include "TestClass2.h"


TossEditor::TossEditor()
{
    editorPreferences.LoadFromFile("EditorPreferences.json");

    auto& tossEngine = TossEngine::GetInstance();
    tossEngine.Init();
    tossEngine.TryCreateWindow(this, editorPreferences.windowSize, "TossEditor", editorPreferences.maximized);
    tossEngine.ReloadScripts(); //recompiles and loads scripts

    ResourceManager& resourceManager = ResourceManager::GetInstance();
    tossEngine.StartCoroutine(resourceManager.loadResourceDesc("Resources/Resources.json"));
    tossEngine.StartCoroutine(resourceManager.createResourcesFromDescs());

    m_projectSettings = std::make_unique<ProjectSettings>();
    m_projectSettings->LoadFromFile("ProjectSettings.json");

    Physics::GetInstance().SetGravity(m_projectSettings->gravity);
    Physics::GetInstance().SetDebug(true);

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
    AudioEngine::GetInstance().Init();


    m_sceneFrameBuffer = std::make_shared<Framebuffer>(tossEngine.GetWindow()->getInnerSize());
    m_gameViewFrameBuffer = std::make_shared<Framebuffer>(tossEngine.GetWindow()->getInnerSize());


    //testClass = new TestClass();
    //json data = JsonUtility::OpenJsonFile("TestClass.json");
    //testClass->Deserialize(data);
    //
    //Debug::Log(ToString(testClass->_intField));


	testClass2 = new TestClass2();
	json data2 = JsonUtility::OpenJsonFile("TestClass2.json");
    testClass2->Deserialize(data2);

	Debug::Log(ToString(testClass2->_intField5));
	Debug::Log(ToString(testClass2->_intField2));
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
            tossEngine.PollEvents();
            if (canUpdateInternal)
            {
                onUpdateInternal();
            }
            //check again as it can change
            if (canUpdateInternal)
            {
                onLateUpdateInternal();
            }
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
    string filePath = editorPreferences.lastKnownOpenScenePath;

    if (!filePath.empty()) // If a file was selected
    {
        auto scene = std::make_shared<Scene>(filePath);
        OpenScene(scene);
    }

    sourceWatcher = new FileWatcher("C++Scripts/");

    scriptWatcherThread = std::thread(&TossEditor::LoadWatchAndCompileScripts, this);

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

void TossEditor::onUpdateInternal()
{
    TossEngine& tossEngine = TossEngine::GetInstance();
    GraphicsEngine& graphicsEngine = GraphicsEngine::GetInstance();
    ResourceManager& resourceManager = ResourceManager::GetInstance();

    FindSceneFiles();

    // delta time
    m_currentTime = tossEngine.GetTime();
    float deltaTime = m_currentTime - m_previousTime;
    m_previousTime = m_currentTime;


    // Accumulate time
    m_accumulatedTime += deltaTime;

    if (m_currentScene)
    {
        InputManager& inputManager = InputManager::GetInstance();
        inputManager.onUpdate();
        if (m_gameRunning)
        {
            Physics::GetInstance().Update(deltaTime);
            while (m_accumulatedTime >= m_fixedTimeStep)
            {
                float fixedDeltaTime = m_currentTime - m_previousFixedUpdateTime;
                m_previousFixedUpdateTime = m_currentTime;
                m_currentScene->onFixedUpdate(fixedDeltaTime);
                m_accumulatedTime -= m_fixedTimeStep;
            }
        }
        // player update
        m_player->Update(deltaTime);
        m_currentScene->onUpdateInternal();

        if (m_gameRunning)
        {
            m_currentScene->onUpdate(deltaTime);
            m_currentScene->onLateUpdate(deltaTime);
        }
        inputManager.onLateUpdate();
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


    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
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
            if (ImGui::MenuItem("Exit", "ESC"))
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

                if (m_currentScene)
                {
                    m_gameRunning = true;
                    m_currentScene->onStart();
                    m_currentScene->onLateStart();
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

                string filePath = editorPreferences.lastKnownOpenScenePath;

                if (!filePath.empty()) // If a file was selected
                {
                    auto scene = std::make_shared<Scene>(filePath);
                    OpenScene(scene);
                }
                selectedSelectable = nullptr;
            }
        }
        ImGui::EndMainMenuBar();
    }

    Debug::DrawConsole();

    ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {
        // Get the available size for the Scene window.
        ImVec2 availSize = ImGui::GetContentRegionAvail();

        // Convert to your Vector2 type (assuming Vector2 takes width and height).
        Vector2 newSize(availSize.x, availSize.y);
        // Resize the scene's framebuffer to match the current window size.
        m_currentScene->onResize(newSize);

        m_gameViewFrameBuffer->onResize(newSize);
        // Now update the scene rendering with the current camera.
        m_currentScene->onGraphicsUpdate(nullptr, m_gameViewFrameBuffer);

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

        if (m_currentScene)
        {
            Vector2 newSize(availSize.x, availSize.y);

            // Resize the scene's framebuffer to match the current window size.
            m_player->getCamera()->setScreenArea(newSize);
            m_currentScene->onResize(newSize);
            m_sceneFrameBuffer->onResize(newSize);

            // Now update the scene rendering with the current camera.
            m_currentScene->onGraphicsUpdate(m_player->getCamera(), m_sceneFrameBuffer);

            // Get the updated texture after rendering.
            ImTextureID sceneViewTexture = (ImTextureID)m_sceneFrameBuffer->RenderTexture->getId();

            ImGui::SetNextItemAllowOverlap();
            // Display the rendered scene scaled to the available region.
            ImGui::Image(sceneViewTexture, availSize, ImVec2{ 0.f, 1.f }, ImVec2{ 1.f, 0.f });

            if (selectedSelectable)
            {
                if (auto gameObject = dynamic_cast<GameObject*>(selectedSelectable))
                {
                    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
                    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, availSize.x, availSize.y);

                    Mat4 cameraView;
                    m_player->getCamera()->getViewMatrix(cameraView);

                    Mat4 projectionMat;
                    m_player->getCamera()->getProjectionMatrix(projectionMat);

                    Mat4 transformMat = gameObject->m_transform.GetMatrix();
                    ImGuizmo::Manipulate(glm::value_ptr(cameraView.value), glm::value_ptr(projectionMat.value), ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::LOCAL, glm::value_ptr(transformMat.value));
                    if (ImGuizmo::IsUsing())
                    {
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
            if (Physics::GetInstance().GetWorld())
            {
                Physics::GetInstance().SetGravity(m_projectSettings->gravity);
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
        int windowSize[2] = { (int)m_playerSettings->windowSize.x, (int)m_playerSettings->windowSize.y };
        if (ImGui::InputInt2("Game Resolution", windowSize))
        {
            m_playerSettings->windowSize.x = windowSize[0];
            m_playerSettings->windowSize.y = windowSize[1];
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
            selectedSelectable->OnInspectorGUI();
        }
        else
        {
            ImGui::Text("Nothing selected.");
        }
        ImGui::End();
    }

    if (ImGui::Begin("Hierarchy"))
    {
        if (ImGui::Button("Add Empty GameObject")) {
            m_currentScene->getObjectManager()->createGameObject<GameObject>();
        }

        // Display root game objects.
        if (m_currentScene != nullptr)
        {
            for (const auto& pair : m_currentScene->getObjectManager()->m_gameObjects)
            {
                // Only show root game objects.
                if (pair.second->m_transform.parent == nullptr)
                {
                    ShowGameObjectNode(pair.second);
                }
            }
        }
        ImGui::End();
    }

    if (renamingGameObject != nullptr)
    {
        ImGui::OpenPopup("Rename GameObject");
    }
    // Place the modal popup outside of any window blocks
    if (ImGui::BeginPopupModal("Rename GameObject", NULL, ImGuiWindowFlags_AlwaysAutoResize))
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

            ImGui::EndPopup();
        }

        std::map<string, ResourcePtr>& resources = resourceManager.GetAllResources();

        for (auto& [uniqueID, resource] : resources) {
            if (uniqueID.empty()) continue;

            bool isSelected = (resource == resourceManager.GetSelectedResource());

            if (ImGui::Selectable(uniqueID.c_str(), isSelected)) {
                selectedSelectable = resource.get();
                resourceManager.SetSelectedResource(resource);
            }

            // Right-click menu for deleting the resource
            if (ImGui::BeginPopupContextItem(("item_context_" + uniqueID).c_str())) {
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
                    break; // Break since resources have changed
                }
                ImGui::EndPopup();
            }
        }
    }
    ImGui::End();

    // In the same frame, open the popup if flagged
    if (openMeshPopupNextFrame) {
        ImGui::OpenPopup("Enter Mesh ID");
        openMeshPopupNextFrame = false;
    }

    // Show the popup to name the shader
    if (ImGui::BeginPopupModal("Enter Mesh ID", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Mesh ID", meshIDBuffer, sizeof(meshIDBuffer));

        if (ImGui::Button("Create")) {
            if (strlen(meshIDBuffer) > 0) {
                std::string meshId = meshIDBuffer;
                std::filesystem::path root = getProjectRoot();
                std::filesystem::path relativeMeshPath = std::filesystem::relative(meshFilepath, root);

                MeshDesc desc;
                desc.filePath = relativeMeshPath.string();

                resourceManager.createMesh(desc, meshId);

                Debug::Log("Created mesh: " + meshId);

                // Reset state
                meshFilepath.clear();
                meshIDBuffer[0] = '\0';

                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            meshFilepath.clear();
            meshIDBuffer[0] = '\0';

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


    if (openPhysicsMaterialNextFrame) {
        ImGui::OpenPopup("Enter Physics Material ID");
        openPhysicsMaterialNextFrame = false;
    }

    // Show the popup to name the shader
    if (ImGui::BeginPopupModal("Enter Physics Material ID", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Physics Material ID", physicsMaterialIDBuffer, sizeof(physicsMaterialIDBuffer));

        if (ImGui::Button("Create")) {
            if (strlen(physicsMaterialIDBuffer) > 0) {
                std::string physicsMaterialID = physicsMaterialIDBuffer;

                PhysicsMaterialDesc desc;
                resourceManager.createPhysicsMaterial(desc, physicsMaterialID);
                Debug::Log("Created physics material: " + physicsMaterialID);

                // Reset state
                physicsMaterialIDBuffer[0] = '\0';

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
    
    
    double RenderTime_End = (double)glfwGetTime();

    // Render to window
    tossEngine.GetWindow()->present();

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();

}

void TossEditor::onLateUpdateInternal()
{
    if (requestDllReload.load())
    {
        Debug::Log("Reloading DLL safely at end of frame...");
        PerformSafeDllReload();
        requestDllReload.store(false);
    }
    if (requestBuild.load())
    {
        Debug::Log("Building player safely at end of frame...");
        PerformSafeBuild();
        requestBuild.store(false);
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
    if (m_currentScene)
    {
        m_currentScene->onQuit();
    }
    m_editorRunning.store(false);
    TossEngine::GetInstance().StartCoroutine(ResourceManager::GetInstance().saveResourcesDescs("Resources/Resources.json"));
    editorPreferences.SaveToFile("EditorPreferences.json");

    json data = testClass->Serialize();
    JsonUtility::SaveJsonFile("TestClass.json", data);

    json data2 = testClass2->Serialize();
    JsonUtility::SaveJsonFile("TestClass2.json", data2);
}

void TossEditor::ShowGameObjectNode(GameObject* gameObject)
{
    ImGuiTreeNodeFlags flags = (gameObject->m_transform.children.empty() ? ImGuiTreeNodeFlags_Leaf : 0);
    if (gameObject == selectedSelectable)
        flags |= ImGuiTreeNodeFlags_Selected;

    bool open = ImGui::TreeNodeEx(gameObject->name.c_str(), flags);
    if (ImGui::IsItemClicked())
    {
        selectedSelectable = gameObject;
    }

    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("DND_GAMEOBJECT", &gameObject, sizeof(GameObject*));
        ImGui::Text("Dragging %s", gameObject->name.c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_GAMEOBJECT"))
        {
            GameObject* droppedGameObject = *(GameObject**)payload->Data;
            if (droppedGameObject != gameObject)
            {
                droppedGameObject->m_transform.SetParent(&gameObject->m_transform);
            }
        }
        ImGui::EndDragDropTarget();
    }

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
            if (gameObject->m_transform.parent)
            {
                gameObject->m_transform.SetParent(nullptr);
            }
            gameObject->Delete();
            if (selectedSelectable == gameObject)
                selectedSelectable = nullptr;
        }
        ImGui::EndPopup();
    }

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

    //TossEngine& tossEngine = TossEngine::GetInstance();
    //tossEngine.UnLoadScripts();

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

    //tossEngine.LoadScripts();
}
void TossEditor::PerformSafeDllReload()
{
    //all not needed yepee but will leave here if i found that it does cause issues
    //Save();
    //if (m_currentScene != nullptr)
    //{
    //    m_currentScene->onQuit();
    //    m_currentScene.reset();
    //    m_currentScene = nullptr;
    //}
    canUpdateInternal = false;

    TossEngine::GetInstance().ReloadScripts();
    m_currentScene->reload();

    canUpdateInternal = true;

    //string filePath = m_projectSettings->lastKnownOpenScenePath;
    //
    //if (!filePath.empty()) // If a file was selected
    //{
    //    auto scene = std::make_shared<Scene>(filePath);
    //    scene->SetWindowFrameBuffer(m_sceneViewFrameBuffer);
    //    OpenScene(scene);
    //}
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

void TossEditor::Save()
{
    m_projectSettings->SaveToFile("ProjectSettings.json");
    m_playerSettings->SaveToFile("PlayerSettings.json");
    if (m_currentScene)
    {
        m_currentScene->Save();
    }
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
            auto scene = std::make_shared<Scene>(relativeSceneFilePath.string());
            OpenScene(scene);
        }
        else {
            Debug::LogWarning("Selected scene file must be inside the project folder.");
        }
    }
}

void TossEditor::OpenScene(shared_ptr<Scene> _scene)
{
    // if there is a current scene, call onQuit
    if (m_currentScene != nullptr)
    {
        m_currentScene->onQuit();
    }


    // set the current scene to the new scene
    m_currentScene = std::move(_scene);
    editorPreferences.lastKnownOpenScenePath = m_currentScene->GetFilePath();
    m_currentScene->onCreate();
    m_currentScene->onCreateLate();
}
