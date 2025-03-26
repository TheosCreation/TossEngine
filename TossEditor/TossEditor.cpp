#include "TossEditor.h"
#include "Window.h"
#include "Game.h"
#include "ProjectSettings.h"
#include "TossPlayerSettings.h"
#include "GraphicsEngine.h"
#include "EditorPlayer.h"
#include "ISelectable.h"
#include "ScriptLoader.h"
#include "FileWatcher.h"
#include <imgui.h>

TossEditor::TossEditor()
{
    editorPreferences.LoadFromFile("EditorPreferences.json");

    auto& tossEngine = TossEngine::GetInstance();
    tossEngine.Init();
    tossEngine.TryCreateWindow(this, editorPreferences.windowSize, "TossEditor", editorPreferences.maximized);

    ResourceManager& resourceManager = ResourceManager::GetInstance();
    TossEngine::GetInstance().StartCoroutine(resourceManager.loadResourceDesc("Resources/Resources.json"));
    TossEngine::GetInstance().StartCoroutine(resourceManager.createResourcesFromDescs());

    m_projectSettings = std::make_unique<ProjectSettings>();
    m_projectSettings->LoadFromFile("ProjectSettings.json");

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


    m_sceneViewFrameBuffer = std::make_shared<Framebuffer>(tossEngine.GetWindow()->getInnerSize());
    m_gameViewFrameBuffer = std::make_shared<Framebuffer>(tossEngine.GetWindow()->getInnerSize());

    scriptWatcherThread = std::thread(&TossEditor::LoadWatchAndCompileScripts, this);
}

TossEditor::~TossEditor()
{
}

void TossEditor::run()
{
    auto& tossEngine = TossEngine::GetInstance();
    //tossEngine.LoadGenericResources();

    onCreate();
    onCreateLate();

    //run funcs while window open
    while (tossEngine.GetWindow()->shouldClose() == false)
    {
        tossEngine.PollEvents();
        if (canUpdateInternal)
        {
            onUpdateInternal();
            onLateUpdateInternal();
        }
    }

    
    onQuit();
    tossEngine.CleanUp();
    if (scriptWatcherThread.joinable())
        scriptWatcherThread.join();
}

void TossEditor::onCreate()
{
    string filePath = m_projectSettings->lastKnownOpenScenePath;

    if (!filePath.empty()) // If a file was selected
    {
        auto scene = std::make_shared<Scene>(filePath);
        scene->SetWindowFrameBuffer(m_sceneViewFrameBuffer);
        OpenScene(scene);
    }

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

    if (m_game != nullptr)
    {
        while (m_accumulatedTime >= m_fixedTimeStep)
        {
            float fixedDeltaTime = m_currentTime - m_previousFixedUpdateTime;
            m_previousFixedUpdateTime = m_currentTime;
            m_game->onFixedUpdate(fixedDeltaTime);
            m_accumulatedTime -= m_fixedTimeStep;
        }

        // player update
        //m_player->Update(deltaTime);
        m_game->onUpdate(deltaTime);
        m_game->onLateUpdate(deltaTime);
    }
    else
    {
        InputManager& inputManager = InputManager::GetInstance();
        inputManager.onUpdate();

        // player update
        m_player->Update(deltaTime);
        if (m_currentScene)
        {
            m_currentScene->onUpdateInternal();
        }

        inputManager.onLateUpdate();
    }


    graphicsEngine.clear(glm::vec4(0, 0, 0, 1)); //clear the existing stuff first is a must
    graphicsEngine.createImGuiFrame();
    ImGui::SetCurrentContext(graphicsEngine.getImGuiContext());

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
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);
    
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    const ImGuiWindowClass* window_class = nullptr;
    
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags, window_class);
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
            if (!m_game && m_currentScene)
            {
                auto scene = std::make_shared<Scene>(m_currentScene->GetFilePath());
                scene->SetWindowFrameBuffer(m_gameViewFrameBuffer);
                m_game = new Game(m_projectSettings);
                m_game->SetScene(scene);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop"))
        {
            if (m_game)
            {
                delete m_game;  // Clean up memory
                m_game = nullptr;
            }
        }
        ImGui::EndMainMenuBar();
    }

    
    ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {
        // Get the available size for the Scene window.
        ImVec2 availSize = ImGui::GetContentRegionAvail();

        // Convert to your Vector2 type (assuming Vector2 takes width and height).
        Vector2 newSize(availSize.x, availSize.y);
        if (m_game != nullptr)
        {
            // Resize the scene's framebuffer to match the current window size.
            if (m_game)
            {
                m_game->onResize(newSize);
            }

            // Now update the scene rendering with the current camera.
            m_game->onGraphicsUpdate();

            // Get the updated texture after rendering.
            ImTextureID gameViewTexture = (ImTextureID)m_gameViewFrameBuffer->RenderTexture->getId();

            // Display the rendered scene scaled to the available region.
            ImGui::Image(gameViewTexture, availSize,
                ImVec2{ 0.f, 1.f },  // UV0
                ImVec2{ 1.f, 0.f }   // UV1 (flipped vertically if needed)
            );
        }
    }
    ImGui::End();

    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {
        // Get the available size for the Scene window.
        ImVec2 availSize = ImGui::GetContentRegionAvail();

        // Convert to your Vector2 type (assuming Vector2 takes width and height).
        Vector2 newSize(availSize.x, availSize.y);
        if (m_currentScene)
        {
            // Resize the scene's framebuffer to match the current window size.
            if (m_currentScene)
            {
                m_player->getCamera()->setScreenArea(newSize);
                m_currentScene->onResize(newSize);
            }

            // Now update the scene rendering with the current camera.
            m_currentScene->onGraphicsUpdate(m_player->getCamera());

            // Get the updated texture after rendering.
            ImTextureID sceneViewTexture = (ImTextureID)m_sceneViewFrameBuffer->RenderTexture->getId();

            // Display the rendered scene scaled to the available region.
            ImGui::Image(sceneViewTexture, availSize,
                ImVec2{ 0.f, 1.f },  // UV0
                ImVec2{ 1.f, 0.f }   // UV1 (flipped vertically if needed)
            );
        }
    }
    ImGui::End();

    if (ImGui::Begin("Settings"))
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
        int windowSize[2] = { m_playerSettings->windowSize.x, m_playerSettings->windowSize.y };
        if (ImGui::InputInt2("Game Resolution", windowSize))
        {
            m_playerSettings->windowSize.x = windowSize[0];
            m_playerSettings->windowSize.y = windowSize[1];
        }


        // --- Build Button ---
        //if (ImGui::Button("Build Project"))
        //{
        //    // Define the absolute paths (with escaped backslashes).
        //    std::string msbuildPath = "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe";
        //    std::string projectFile = "C:\\Users\\Theo\\TossEngine\\TossPlayer\\TossPlayer.vcxproj";
        //    std::string configuration = "/p:Configuration=Release";
        //    // Use a double trailing backslash to avoid escaping the closing quote.
        //    std::string solutionDirProp = " /p:SolutionDir=\"C:\\Users\\Theo\\TossEngine\\\\\"";
        //
        //    // Construct the command string using cmd /c and proper quoting.
        //    std::string command = "cmd /c \"\"" + msbuildPath + "\" \"" + projectFile + "\" " + configuration + solutionDirProp + "\"";
        //
        //    Debug::Log("Starting build: " + command);
        //    int result = system(command.c_str());
        //
        //    if (result == 0)
        //    {
        //        Debug::Log("Build succeeded.");
        //    }
        //    else
        //    {
        //        Debug::Log("Build failed with error code: " + std::to_string(result));
        //    }
        //}
    }
    ImGui::End();


    if (ImGui::Begin("Inspector"))
    {
        if (selectedSelectable)
        {
            // Display the selected object's name and transform.
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
        // Right-click context menu on the window background
        if (ImGui::BeginPopupContextWindow("Hierarchy Context"))
        {
            if (ImGui::MenuItem("Add Empty GameObject"))
            {
                m_currentScene->getObjectManager()->createGameObject<GameObject>();
            }
            ImGui::EndPopup();
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


    static std::string shaderVertPath;
    static std::string shaderFragPath;
    static char shaderIDBuffer[256] = "";
    static bool openShaderPopupNextFrame = false;
    if (ImGui::Begin("Assets")) {
        const auto& resources = resourceManager.GetAllResources();

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
                    resourceManager.DeleteResource(resource);
                    if (isSelected) {
                        resourceManager.SetSelectedResource(nullptr);
                        selectedSelectable = nullptr;
                    }
                    ImGui::CloseCurrentPopup();
                    break; // Break since resources have changed
                }
                ImGui::EndPopup();
            }
        }

        // Right-click on empty space to create new resource
        if (ImGui::BeginPopupContextWindow("create_resource_context", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem("Create Texture2D")) {
                std::string textureFilePath = TossEngine::GetInstance().openFileDialog("*.png;*.jpg;*.jpeg;*.bmp");

                if (!textureFilePath.empty()) {
                    fs::path selectedPath = textureFilePath;
                    fs::path projectRoot = getProjectRoot();
                    fs::path relativePath;

                    // Ensure the selected path is within the project root
                    if (fs::equivalent(selectedPath.root_name(), projectRoot.root_name()) &&
                        selectedPath.string().find(projectRoot.string()) == 0)
                    {
                        relativePath = fs::relative(selectedPath, projectRoot);
                        Debug::Log(relativePath.string());
                        resourceManager.createTexture2DFromFile(relativePath.string());
                    }
                    else
                    {
                        Debug::LogWarning("Selected texture must be inside the project folder.");
                    }
                }
            }
            if (ImGui::MenuItem("Create Mesh")) {

                std::string meshObjFilePath = TossEngine::GetInstance().openFileDialog("*.obj;");

                if (!meshObjFilePath.empty()) {
                    fs::path selectedPath = meshObjFilePath;
                    fs::path projectRoot = getProjectRoot();
                    fs::path relativePath;

                    // Ensure the selected path is within the project root
                    if (fs::equivalent(selectedPath.root_name(), projectRoot.root_name()) &&
                        selectedPath.string().find(projectRoot.string()) == 0)
                    {
                        relativePath = fs::relative(selectedPath, projectRoot);
                        resourceManager.createMeshFromFile(relativePath.string());
                    }
                    else
                    {
                        Debug::LogWarning("Selected mesh must be inside the project folder.");
                    }
                }
            }

            if (ImGui::MenuItem("Create Shader")) {
                shaderVertPath = TossEngine::GetInstance().openFileDialog("*.vert");
                if (!shaderVertPath.empty()) {
                    shaderFragPath = TossEngine::GetInstance().openFileDialog("*.frag");
                    if (!shaderFragPath.empty()) {
                        openShaderPopupNextFrame = true; // trigger for next frame
                    }
                }
            }
            ImGui::EndPopup();
        }
    }
    ImGui::End();

    // In the same frame, open the popup if flagged
    if (openShaderPopupNextFrame) {
        ImGui::OpenPopup("Enter Shader ID");
        openShaderPopupNextFrame = false;
    }

    // Show the popup to name the shader
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

    Debug::DrawConsole();

    graphicsEngine.renderImGuiFrame();
    
    
    double RenderTime_End = (double)glfwGetTime();
    
    // Render to window
    tossEngine.GetWindow()->present();
}

void TossEditor::onLateUpdateInternal()
{
    if (requestDllReload)
    {
        Debug::Log("Reloading DLL safely at end of frame...");
        PerformSafeDllReload();
        requestDllReload = false;
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
    m_editorRunning = false;
    TossEngine::GetInstance().StartCoroutine(ResourceManager::GetInstance().saveResourcesDescs("Resources/Resources.json"));
    editorPreferences.SaveToFile("EditorPreferences.json");
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
    FileWatcher sourceWatcher("C++Scripts/");

    while (m_editorRunning) {
        if (sourceWatcher.hasChanged()) {
            Debug::Log("Scripts DLL changed. Marking for reload...");
            requestDllReload = true;  // atomic or thread-safe bool
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void TossEditor::PerformSafeDllReload()
{
    //all not needed yepee but will leave here if i found that it does cause issues
    //Save();
    //ComponentRegistry::GetInstance().CleanUp();
    //if (m_currentScene != nullptr)
    //{
    //    m_currentScene->onQuit();
    //    m_currentScene.reset();
    //    m_currentScene = nullptr;
    //}
    canUpdateInternal = false;

    TossEngine::GetInstance().ReloadDLL();

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
    string filePath = TossEngine::GetInstance().openFileDialog("*.json");

    if (!filePath.empty()) // If a file was selected
    {
        auto scene = std::make_shared<Scene>(filePath);
        OpenScene(scene);
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
    m_projectSettings->lastKnownOpenScenePath = m_currentScene->GetFilePath();
    m_currentScene->onCreate();
    m_currentScene->onCreateLate();
}
