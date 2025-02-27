#include "TossEditor.h"
#include "TossEngine.h"
#include "Window.h"
#include "Game.h"
#include "ProjectSettings.h"
#include "GraphicsEngine.h"
#include "InputManager.h"

TossEditor::TossEditor()
{
    m_projectSettings = std::make_unique<ProjectSettings>();
    m_projectSettings->LoadFromFile("ProjectSettings.json");

    Vector2 windowSize = Vector2(800, 800);

    auto& tossEngine = TossEngine::GetInstance();
    tossEngine.Init();
    tossEngine.CreateWindowOrChangeOwner(this, windowSize, "TossEditor");

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.Init(m_projectSettings);
    graphicsEngine.setViewport(windowSize);
    graphicsEngine.setDepthFunc(DepthType::Less);
    graphicsEngine.setBlendFunc(BlendType::SrcAlpha, BlendType::OneMinusSrcAlpha);
    graphicsEngine.setFaceCulling(CullType::BackFace);
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
    graphicsEngine.setScissorSize(Rect(200, 200, 400, 300));
    graphicsEngine.setMultiSampling(true);

    auto& inputManager = InputManager::GetInstance();
    inputManager.Init(m_projectSettings);
    inputManager.setScreenArea(windowSize);

    LightManager::GetInstance().Init();
}

TossEditor::~TossEditor()
{
}

void TossEditor::run()
{
    auto& tossEngine = TossEngine::GetInstance();
    onCreate();
    onCreateLate();

    //run funcs while window open
    while (tossEngine.GetWindow()->shouldClose() == false)
    {
        tossEngine.PollEvents();
        onUpdateInternal();
    }

    game = new Game(m_projectSettings);
    game->run();
    
    onQuit();
    tossEngine.CleanUp();
}

void TossEditor::onCreate()
{
    auto scene = std::make_shared<Scene>();
    OpenScene(scene);
}

void TossEditor::onCreateLate()
{
}

void TossEditor::onUpdateInternal()
{
    auto& tossEngine = TossEngine::GetInstance();

    auto& inputManager = InputManager::GetInstance();
    inputManager.onUpdate();

    // delta time
    m_currentTime = tossEngine.GetCurrentTime();
    float deltaTime = m_currentTime - m_previousTime;
    m_previousTime = m_currentTime;

    // player update

    inputManager.onLateUpdate();

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.clear(glm::vec4(0, 0, 0, 1)); //clear the existing stuff first is a must
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    m_currentScene->onGraphicsUpdate(); //Render the scene
    
    ImGui::Begin("Settings menu lol");
    ImGui::Text("These are your options");
    
    //static const char* items[]{ "Deferred Rendering","Forward" }; static int Selecteditem = (int)m_projectSettings->renderingPath;
    //if (ImGui::Combo("Rendering Path", &Selecteditem, items, IM_ARRAYSIZE(items)))
    //{
    //    RenderingPath selectedPath = static_cast<RenderingPath>(Selecteditem);
    //    Debug::Log("Rendering Path changed to option: " + ToString(selectedPath));
    //    graphicsEngine.setRenderingPath(selectedPath);
    //    m_projectSettings->renderingPath = selectedPath;
    //}
    
    //if (ImGui::Button("Play"))
    //{
    //    m_isRunning = true;
    //}
    //
    //if (ImGui::Button("Stop"))
    //{
    //    m_isRunning = false;
    //}
    
    ImGui::End();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    double RenderTime_End = (double)glfwGetTime();
    
    // Render to window
    tossEngine.GetWindow()->present();
}

void TossEditor::onQuit()
{
}

void TossEditor::onResize(Vector2 size)
{
    Resizable::onResize(size);

}

void TossEditor::save()
{
    m_projectSettings->SaveToFile("ProjectSettings.json");
}

void TossEditor::OpenScene(shared_ptr<Scene> _scene)
{
    // if there is a current scene, call onQuit
    if (m_currentScene != nullptr)
    {
        m_currentScene->onQuit();
        LightManager::GetInstance().reset();
        ResourceManager::GetInstance().ClearInstancesFromMeshes();
    }
    // set the current scene to the new scene
    m_currentScene = std::move(_scene);
    m_currentScene->onCreate();
    m_currentScene->onCreateLate();
}
