#include "TossPlayer.h"
#include "TossEngine.h"
#include "Window.h"
#include "Game.h"
#include "TossPlayerSettings.h"
#include "GraphicsEngine.h"
#include "ISelectable.h"
#include <imgui.h>

TossPlayer::TossPlayer()
{
    m_playerSettings = std::make_unique<TossPlayerSettings>();
    if (!m_playerSettings->LoadFromFile("PlayerSettings.json"))
    {
        abort = true;
    }

    Vector2 windowSize = m_playerSettings->windowSize;


    auto& tossEngine = TossEngine::GetInstance();
    tossEngine.Init();
    tossEngine.TryCreateWindow(this, windowSize, "TossPlayer");
    tossEngine.LoadScripts();

    Physics::GetInstance().SetGravity(m_playerSettings->gravity);

    AudioEngine::GetInstance().Init();

    ResourceManager& resourceManager = ResourceManager::GetInstance();
    tossEngine.StartCoroutine(resourceManager.loadResourceDesc("Resources/Resources.json"));
    tossEngine.StartCoroutine(resourceManager.createResourcesFromDescs());

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.Init(m_playerSettings);
    graphicsEngine.setViewport(windowSize);
    graphicsEngine.setDepthFunc(DepthType::Less);
    graphicsEngine.setBlendFunc(BlendType::SrcAlpha, BlendType::OneMinusSrcAlpha);
    graphicsEngine.setFaceCulling(CullType::BackFace);
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
    graphicsEngine.setMultiSampling(true);

    auto& inputManager = InputManager::GetInstance();
    inputManager.Init(m_playerSettings);
    inputManager.setScreenArea(windowSize);

    GeometryBuffer::GetInstance().Init(windowSize);
}

TossPlayer::~TossPlayer()
{
}

void TossPlayer::run()
{
    auto& tossEngine = TossEngine::GetInstance();
    if (abort)
    {
        tossEngine.GetWindow()->close(); 
        return;
    }

    tossEngine.LoadGenericResources();

    onCreate();
    onCreateLate();

    //run funcs while window open
    while (tossEngine.GetWindow()->shouldClose() == false)
    {
        tossEngine.PollEvents();
        onUpdateInternal();
    }

    
    onQuit();
    tossEngine.CleanUp();
}

void TossPlayer::onCreate()
{
    auto scene = std::make_shared<Scene>(m_playerSettings->firstSceneToOpen);
    m_game = new Game(m_playerSettings);
    m_game->SetScene(scene);
}

void TossPlayer::onCreateLate()
{
}


void TossPlayer::onUpdateInternal()
{
    TossEngine& tossEngine = TossEngine::GetInstance();
    GraphicsEngine& graphicsEngine = GraphicsEngine::GetInstance();

    // delta time
    m_currentTime = tossEngine.GetTime();
    float deltaTime = m_currentTime - m_previousTime;
    m_previousTime = m_currentTime;

    // Accumulate time
    m_accumulatedTime += deltaTime;

    while (m_accumulatedTime >= m_fixedTimeStep)
    {
        float fixedDeltaTime = m_currentTime - m_previousFixedUpdateTime;
        m_previousFixedUpdateTime = m_currentTime;
        m_game->onFixedUpdate(fixedDeltaTime);
        m_accumulatedTime -= m_fixedTimeStep;
    }
    Physics::GetInstance().Update(deltaTime);
    m_game->onUpdate(deltaTime);
    m_game->onLateUpdate(deltaTime);

    graphicsEngine.clear(glm::vec4(0, 0, 0, 1)); //clear the existing stuff first is a must
    m_game->onGraphicsUpdate();
    
    // Render to window
    tossEngine.GetWindow()->present();
}

void TossPlayer::onQuit()
{
    m_game->getScene()->onQuit();
}

void TossPlayer::onResize(Vector2 size)
{
    Resizable::onResize(size);

    m_game->onResize(size);

    GeometryBuffer::GetInstance().Resize(size);
}
