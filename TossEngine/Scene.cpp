/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Scene.h
Description : Base class representing a game scene, providing essential methods for rendering and updating the game logic.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "Scene.h"
#include "GameObjectManager.h"
#include "GraphicsEngine.h"
#include "GeometryBuffer.h"
#include "AudioEngine.h"
#include "ComponentRegistry.h"
#include "MeshRenderer.h"
#include "Rigidbody.h"
#include "Image.h"
#include "Material.h"
#include "Ship.h"
#include "PointLight.h"
#include "TossEngine.h"
#include "Skybox.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Physics.h"

Scene::Scene(const string& filePath)
{
    m_filePath = filePath;

    auto& tossEngine = TossEngine::GetInstance(); 

    m_lightManager = std::make_unique<LightManager>();
    m_postProcessingFramebuffer = std::make_shared<Framebuffer>(tossEngine.GetWindow()->getInnerSize());
    m_gameObjectManager = std::make_unique<GameObjectManager>(this);

    m_SSRQ = std::make_unique<Image>();
    m_SSRQ->SetSize({ -2.0f, 2.0f });
}

Scene::Scene(const Scene& other)
{
    auto& tossEngine = TossEngine::GetInstance();


    m_lightManager = std::make_unique<LightManager>();
    m_postProcessingFramebuffer = std::make_shared<Framebuffer>(tossEngine.GetWindow()->getInnerSize());

    // Copy GameObjectManager (requires a proper copy constructor or Clone() function)
     
    m_gameObjectManager = std::make_unique<GameObjectManager>(this);
    //m_gameObjectManager = std::make_unique<GameObjectManager>(*other.m_gameObjectManager);
    m_SSRQ = std::make_unique<Image>();
    m_SSRQ->SetSize({ -2.0f, 2.0f });
}

Scene::~Scene()
{
}

void Scene::reload()
{
    m_gameObjectManager->clearGameObjects();
    m_lightManager->clearLights();
    onCreate(); 
    onCreateLate();
}

void Scene::onCreate()
{
    auto& graphicsEngine = GraphicsEngine::GetInstance();

    auto& resourceManager = ResourceManager::GetInstance();
    TossEngine::GetInstance().StartCoroutine(resourceManager.loadResourceDesc("Resources/Resources.json"));

    m_deferredSSRQMaterial = resourceManager.getMaterial("DeferredSSRQMaterial");
    m_postProcessSSRQMaterial = resourceManager.getMaterial("PostProcessSSRQMaterial");

    m_gameObjectManager->loadGameObjectsFromFile(m_filePath);

    //MeshDesc mesh;
    //mesh.filePath = "Resources/Meshes/SM_Prop_Statue_01.obj";
    //MeshPtr statueMesh = resourceManager.createMesh(mesh, "Resources/Meshes/SM_Prop_Statue_01.obj");
    //float spacing = 50.0f;
    //for (int row = -4; row < 4; ++row) {
    //    for (int col = -4; col < 4; ++col) {
    //        // Calculate the position of the current tree based on the grid and spacing
    //        Vector3 position = Vector3(col * spacing, 0, row * spacing);
    //
    //        if (position == Vector3(0.0f)) break;
    //
    //        // Generate random rotation angles
    //        float angleY = randomNumber(360.0f);
    //
    //        // Add the tree instance with random rotations
    //        statueMesh->addInstance(position, Vector3(0.2f), Vector3(0, angleY, 0));
    //    }
    //}
    //Init instance buffer
    //statueMesh->initInstanceBuffer();


    //defaultFullscreenShader = resourceManager.createShader({
    //        "ScreenQuad",
    //        "QuadShader"
    //    },
    //    "DefaultFullscreenShader"
    //);
    //
    //ssrQuadLightingShader = resourceManager.createShader({
    //        "ScreenQuad",
    //        "SSRLightingShader"
    //    },
    //    "SSQLightingShader"
    //);

    //m_postProcessSSRQ->onCreate();
    //m_postProcessSSRQ->setShader(defaultFullscreenShader);
    //m_postProcessSSRQ->setTexture(m_postProcessingFramebuffer->RenderTexture);
    //ShaderPtr skyboxShader = resourceManager.createShader({
    //        "SkyBoxShader",
    //        "SkyBoxShader"
    //    },
    //    "SkyBoxShader"
    //);

    //m_solidColorMeshShader = resourceManager.createShader({
    //        "SolidColorMesh",
    //        "SolidColorMesh"
    //    },
    //    "SolidColorMeshShader"
    //);

    //m_shadowShader = resourceManager.createShader({
    //        "ShadowShader",
    //        "ShadowShader"
    //    },
    //    "ShadowShader"
    //);
    //
    //m_shadowInstancedShader = resourceManager.createShader({
    //        "ShadowShaderInstanced",
    //        "ShadowShader"
    //    },
    //    "ShadowShaderInstancedShader"
    //);
    //m_meshGeometryShader = resourceManager.createShader({
    //        "MeshShader",
    //        "GeometryPass"
    //    },
    //    "GeometryPassMeshShader"
    //);
    //m_instancedmeshGeometryShader = resourceManager.createShader({
    //        "InstancedMesh",
    //        "GeometryPass"
    //    },
    //    "GeometryPassInstancedMeshShader"
    //);

    //m_terrainGeometryShader = resourceManager.createShader({
    //        "TerrainShader",
    //        "GeometryPassTerrian"
    //    },
    //    "GeometryPassTerrianShader"
    //);

    //m_particleSystemShader = resourceManager.createShader({
    //        "ParticleSystem",
    //        "ParticleSystem"
    //    },
    //    "ParticleSystem"
    //);

    //m_computeShader = resourceManager.createComputeShader("ComputeParticles");

    //m_meshLightingShader = graphicsEngine.createShader({
    //        "MeshShader",
    //        "MeshLightingShader"
    //    });

    // create a cube map texture and set the texture of the skybox to the cubemap texture
    //std::vector<std::string> skyboxCubeMapTextureFilePaths;
    //skyboxCubeMapTextureFilePaths.push_back("Resources/Textures/RedEclipse/Right.png");
    //skyboxCubeMapTextureFilePaths.push_back("Resources/Textures/RedEclipse/Left.png");
    //skyboxCubeMapTextureFilePaths.push_back("Resources/Textures/RedEclipse/Top.png");
    //skyboxCubeMapTextureFilePaths.push_back("Resources/Textures/RedEclipse/Bottom.png");
    //skyboxCubeMapTextureFilePaths.push_back("Resources/Textures/RedEclipse/Back.png");
    //skyboxCubeMapTextureFilePaths.push_back("Resources/Textures/RedEclipse/Front.png");
    //TextureCubeMapPtr skyBoxTexture = resourceManager.createCubeMapTextureFromFile(skyboxCubeMapTextureFilePaths);
    

    //Texture2DPtr heightMapTexture = resourceManager.createTexture2DFromFile("Resources/Textures/Heightmap0.jpg");
    //Texture2DPtr shipReflectiveMap = resourceManager.createTexture2DFromFile("Resources/Textures/ReflectionMap_White.png");
    //Texture2DPtr sciFiSpaceTexture2D = resourceManager.createTexture2DFromFile("Resources/Textures/PolygonSciFiSpace_Texture_01_A.png");
    //Texture2DPtr ancientWorldsTexture2D = resourceManager.createTexture2DFromFile("Resources/Textures/PolygonAncientWorlds_Texture_01_A.png");
    //Texture2DPtr grassTexture = resourceManager.createTexture2DFromFile("Resources/Textures/Terrain/stone.png");
    //Texture2DPtr dirtTexture = resourceManager.createTexture2DFromFile("Resources/Textures/Terrain/dirt.png");
    //Texture2DPtr stoneTexture = resourceManager.createTexture2DFromFile("Resources/Textures/Terrain/stone.png");
    //Texture2DPtr snowTexture = resourceManager.createTexture2DFromFile("Resources/Textures/Terrain/snow.png");

    //MeshPtr fighterShip = resourceManager.createMeshFromFile("Resources/Meshes/Space/SM_Ship_Fighter_02.obj");

    //ShaderPtr instancedMeshShader = resourceManager.createShader({
    //        "InstancedMesh",
    //        "MeshShader"
    //    },
    //    "InstancedMeshShader"
    //);

    //ShaderPtr terrainShader = resourceManager.createShader({
    //        "TerrainShader",
    //        "TerrainShader"
    //    },
    //    "TerrainShader"
    //);
    //MaterialPtr skyboxMaterial = resourceManager.createMaterial("SkyBoxShader", "SkyboxMatrial");

    //HeightMapInfo buildInfo = { "Resources/Heightmaps/Heightmap0.raw", 256, 256, 4.0f };
    //HeightMapPtr heightmap = resourceManager.createHeightMap(buildInfo);
    //
    //m_terrain = m_gameObjectManager->createGameObject<TerrainGameObject>();
    //m_terrain->generateTerrainMesh(heightmap);
    //m_terrain->m_transform.position = Vector3(0, -20, 0);
    //m_terrain->setTexture(grassTexture);
    //m_terrain->setTexture1(dirtTexture);
    //m_terrain->setTexture2(stoneTexture);
    //m_terrain->setTexture3(snowTexture);
    //m_terrain->setHeightMap(heightMapTexture);
    //m_terrain->setShader(terrainShader);
    //m_terrain->setShadowShader(m_shadowShader);
    //m_terrain->setGeometryShader(m_terrainGeometryShader);
    //
    
    //Creating skybox object
   //{
   //    auto skyboxObject = m_gameObjectManager->createGameObject<GameObject>();
   //    auto skybox = skyboxObject->addComponent<Skybox>();
   //    skybox->setMesh(resourceManager.getMesh("Resources/Meshes/cube.obj"));
   //    skybox->setTexture(skyBoxTexture);
   //    skybox->SetMaterial(skyboxMaterial);
   //}
   //{
   //    auto ship = m_gameObjectManager->createGameObject<GameObject>();
   //    ship->m_transform.scale = Vector3(0.05f);
   //    ship->m_transform.position = Vector3(0.0f, 20.0f, 0.0f);
   //    auto renderer = ship->addComponent<MeshRenderer>();
   //    renderer->SetShininess(0.0f);
   //    renderer->SetTexture(sciFiSpaceTexture2D);
   //    renderer->SetShader(resourceManager.getShader("MeshShader"));
   //    renderer->SetMesh(fighterShip);
   //    renderer->SetReflectiveMapTexture(shipReflectiveMap);
   //    renderer->SetShadowShader(resourceManager.getShader("ShadowShader"));
   //    renderer->SetGeometryShader(resourceManager.getShader("GeometryPassMeshShader"));
   //
   //  ship->addComponent("DestroyObjectWithTime");
   //}
   //float pointLightSpacing = 30.0f;
   //// Initialize 2 point lights
   //for (int i = 0; i < 4; i++) {
   //    // Calculate the position based on row and column, center the grid around (0,0)
   //    float xPosition = i * pointLightSpacing; // Center horizontally
   //    float yPosition = 15.0f; // Fixed Y position
   //    float zPosition = 0;
   //
   //    // Randomly set color to either red or blue
   //    int randomColorChoice = (int)randomNumber(2.0f); // Generates 0 or 1
   //    Vector3 lightColor = (randomColorChoice == 0) ? Color::Red * 2.0f : Color::Blue * 2.0f;
   //
   //    // Create a new point light GameObject
   //    auto pointLightObject = m_gameObjectManager->createGameObject<GameObject>();
   //    pointLightObject->m_transform.position = Vector3(xPosition, yPosition, zPosition);
   //    pointLightObject->m_transform.scale = Vector3(3.0f);
   //    
   //    MeshRenderer* meshRenderer = pointLightObject->addComponent<MeshRenderer>();
   //    meshRenderer->SetMesh(resourceManager.getMesh("Resources/Meshes/sphere.obj"));
   //    meshRenderer->SetShader(meshShader);
   //    meshRenderer->SetShadowShader(m_shadowShader);
   //    meshRenderer->SetGeometryShader(m_meshGeometryShader);
   //    meshRenderer->SetAlpha(0.5f);
   //    meshRenderer->SetColor(lightColor);
   //
   //    PointLight* pointLight = pointLightObject->addComponent<PointLight>();
   //    pointLight->SetColor(lightColor);
   //}
   //
   // 
   //Creating instanced tree obj
   //{
   //    auto statues = m_gameObjectManager->createGameObject<GameObject>();
   //    //statues->addCSharpComponent("Ship");
   //    auto renderer = statues->addComponent<MeshRenderer>();
   //    renderer->SetShininess(32.0f);
   //    renderer->SetTexture(ancientWorldsTexture2D);
   //    renderer->SetShader(instancedMeshShader);
   //    renderer->SetMesh(statueMesh);
   //    renderer->SetReflectiveMapTexture(shipReflectiveMap);
   //    renderer->SetShadowShader(m_shadowInstancedShader);
   //    renderer->SetGeometryShader(m_instancedmeshGeometryShader);
   //}
   //
   //for (int i = 0; i < 30; ++i) {
   //    auto physicsSphere = m_gameObjectManager->createGameObject<GameObject>();
   //
   //    // Adjusting position based on index to avoid overlapping spheres
   //    float offset = i * 8.0f; // Adjust this to control the distance between the spheres
   //    physicsSphere->m_transform.position = Vector3(0, 10 + offset, 0.1f);
   //    physicsSphere->m_transform.scale = Vector3(3.0f);
   //
   //    // Mesh and shader setup
   //    auto meshRenderer = physicsSphere->addComponent<MeshRenderer>();
   //    meshRenderer->SetColor(Color::Black);
   //    meshRenderer->SetMesh(resourceManager.getMesh("Resources/Meshes/sphere.obj"));
   //    meshRenderer->SetShader(resourceManager.getShader("MeshShader"));
   //    meshRenderer->SetShadowShader(m_shadowShader);
   //    meshRenderer->SetGeometryShader(m_meshGeometryShader);
   //
   //    // Rigidbody setup with sphere collider
   //    auto rb = physicsSphere->addComponent<Rigidbody>();
   //    rb->SetSphereCollider(3.0f);
   //}
   //
   //{
   //    // Create the ground
   //    auto physicsCube = m_gameObjectManager->createGameObject<GameObject>();
   //    physicsCube->m_transform.position = Vector3(0, -5, 0);
   //    physicsCube->m_transform.scale = Vector3(100.0f, 0.5f, 100.0f);
   //
   //    auto meshRenderer = physicsCube->addComponent<MeshRenderer>();
   //    meshRenderer->SetColor(Color::White);
   //    meshRenderer->SetMesh(resourceManager.getMesh("Resources/Meshes/cube.obj"));
   //    meshRenderer->SetShader(resourceManager.getShader("MeshShader"));
   //    meshRenderer->SetShadowShader(m_shadowShader);
   //    meshRenderer->SetGeometryShader(m_meshGeometryShader);
   //
   //    auto rb = physicsCube->addComponent<Rigidbody>();
   //    rb->SetBoxCollider(Vector3(100.0f, 0.5f, 100.0f));
   //    rb->SetBodyType(BodyType::Static);
   //}
   //
   //// Create four walls to hold the balls
   //{
   //    // Wall dimensions
   //    float wallHeight = 10.0f; // Height of the walls
   //    float wallThickness = 0.2f; // Thickness of the walls
   //    float areaSize = 100.0f; // Size of the area (assuming a square area)
   //
   //    // Wall positions
   //    Vector3 wallPositions[] = {
   //        Vector3(0, wallHeight / 2 - 5, areaSize / 2), // Front wall
   //        Vector3(0, wallHeight / 2 - 5, -areaSize / 2), // Back wall
   //        Vector3(areaSize / 2, wallHeight / 2 - 5, 0),  // Right wall
   //        Vector3(-areaSize / 2, wallHeight / 2 - 5, 0)  // Left wall
   //    };
   //
   //    // Wall scales
   //    Vector3 wallScales[] = {
   //        Vector3(areaSize, wallHeight, wallThickness), // Front and back walls
   //        Vector3(wallThickness, wallHeight, areaSize)  // Right and left walls
   //    };
   //
   //    for (int i = 0; i < 4; ++i) {
   //        auto wall = m_gameObjectManager->createGameObject<GameObject>();
   //        wall->m_transform.position = wallPositions[i];
   //        wall->m_transform.scale = wallScales[i < 2 ? 0 : 1]; // Use appropriate scale for front/back vs right/left walls
   //
   //        auto meshRenderer = wall->addComponent<MeshRenderer>();
   //        meshRenderer->SetColor(Color::Gray); // Set a different color for walls
   //        meshRenderer->SetMesh(resourceManager.getMesh("Resources/Meshes/cube.obj"));
   //        meshRenderer->SetShader(resourceManager.getShader("SolidColorMeshShader"));
   //        meshRenderer->SetShadowShader(m_shadowShader);
   //        meshRenderer->SetGeometryShader(m_meshGeometryShader);
   //
   //        auto rb = wall->addComponent<Rigidbody>();
   //        rb->SetBoxCollider(wall->m_transform.scale); // Collider size matches the wall scale
   //        rb->SetBodyType(BodyType::Static); // Walls are static
   //    }
   //}

}

void Scene::onStart()
{
    m_gameObjectManager->onStart();
}

void Scene::onLateStart()
{
    m_gameObjectManager->onLateStart();
}

void Scene::onCreateLate()
{
    auto& tossEngine = TossEngine::GetInstance();
    for (auto& camera : m_gameObjectManager->getCameras())
    {
        // Set the screen area for all cameras
        camera->setScreenArea(tossEngine.GetWindow()->getInnerSize());
    }
}


void Scene::onUpdate(float deltaTime)
{
    if(deltaTime <= 0.0f) return;

    m_gameObjectManager->onUpdate(deltaTime);
}

void Scene::onUpdateInternal()
{
    m_gameObjectManager->onUpdateInternal();
}

void Scene::onFixedUpdate(float fixedDeltaTime)
{
    if (fixedDeltaTime <= 0.0f) return;

    m_gameObjectManager->onFixedUpdate(fixedDeltaTime); 
}

void Scene::onLateUpdate(float deltaTime)
{
    m_gameObjectManager->onLateUpdate(deltaTime);
}

void Scene::onGraphicsUpdate(Camera* cameraToRenderOverride)
{
    auto& tossEngine = TossEngine::GetInstance();
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.clear(glm::vec4(0, 0, 0, 1));

    // Populate the uniform data struct
    uniformData.currentTime = tossEngine.GetTime();

    if (cameraToRenderOverride != nullptr)
    {
        cameraToRenderOverride->getViewMatrix(uniformData.viewMatrix);
        cameraToRenderOverride->getProjectionMatrix(uniformData.projectionMatrix);
        uniformData.cameraPosition = cameraToRenderOverride->getPosition();
        m_lightManager->setSpotlightPosition(uniformData.cameraPosition);
        m_lightManager->setSpotlightDirection(cameraToRenderOverride->getFacingDirection());
    }
    else
    {
        // get the camera data from a camera in scene
        for (auto& camera : m_gameObjectManager->getCameras())
        {
            if (camera->getCameraType() == CameraType::Perspective)
            {
                camera->getViewMatrix(uniformData.viewMatrix);
                camera->getProjectionMatrix(uniformData.projectionMatrix);
                uniformData.cameraPosition = camera->getPosition();
                m_lightManager->setSpotlightPosition(uniformData.cameraPosition);
                m_lightManager->setSpotlightDirection(camera->getFacingDirection());
            }
            else
            {
                camera->getViewMatrix(uniformData.uiViewMatrix);
                camera->getProjectionMatrix(uniformData.uiProjectionMatrix);
            }
        }
    }
    
    // Example of Defered Rendering Pipeline
    if (graphicsEngine.getRenderingPath() == RenderingPath::Deferred)
    {
        //Geometry Pass
        auto& geometryBuffer = GeometryBuffer::GetInstance();
        geometryBuffer.Bind();
        m_gameObjectManager->Render(uniformData);
        geometryBuffer.UnBind();


        // Shadow Pass: Render shadows for directional lights
        for (uint i = 0; i < m_lightManager->getDirectionalLightCount(); i++)
        {
            m_lightManager->BindShadowMap(i);
            m_gameObjectManager->onShadowPass(i); // Render shadow maps
            m_lightManager->UnBindShadowMap(i);
        }
        graphicsEngine.setViewport(tossEngine.GetWindow()->getInnerSize());

        // Lighting Pass: Apply lighting using G-buffer data
        // Populate the shader with geometry buffer information for the lighting pass
        ShaderPtr shader = m_deferredSSRQMaterial->GetShader();
        graphicsEngine.setShader(shader);
        GeometryBuffer::GetInstance().PopulateShader(shader);

        // Apply lighting settings using the LightManager
        m_lightManager->applyLighting(shader);

        // Apply shadows
        m_lightManager->applyShadows(shader);

        m_postProcessingFramebuffer->Bind();

        // Render the screenspace quad using the lighting, shadow and geometry data
        m_SSRQ->SetMaterial(m_deferredSSRQMaterial);
        m_SSRQ->Render(uniformData, RenderingPath::Forward);

        geometryBuffer.WriteDepth(m_postProcessingFramebuffer->getId());

        // Render the transparent objects after
        m_gameObjectManager->onTransparencyPass(uniformData);
        m_gameObjectManager->onSkyboxPass(uniformData);

        m_postProcessingFramebuffer->UnBind();
    }
    
    // Example of Forward Rendering Pipeline
    if (graphicsEngine.getRenderingPath() == RenderingPath::Forward)
    {
        for (uint i = 0; i < m_lightManager->getDirectionalLightCount(); i++)
        {
            //Shadow Pass
            m_lightManager->BindShadowMap(i);
            m_gameObjectManager->onShadowPass(i);
            m_lightManager->UnBindShadowMap(i);
        }
        graphicsEngine.setViewport(tossEngine.GetWindow()->getInnerSize());

        m_postProcessingFramebuffer->Bind();

        // trying to get debug to show physics
        //graphicsEngine.setFaceCulling(CullType::BackFace);
        //graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
        //graphicsEngine.setDepthFunc(DepthType::Less);
        //graphicsEngine.setShader(shader);
        //
        //
        //// Render the physics debug
        //rp3d::DebugRenderer& debugRenderer = m_PhysicsWorld->getDebugRenderer();
        //auto debugLines = debugRenderer.getLines();
        //
        //// Retrieve the instance of the graphics engine
        //graphicsEngine.setVertexArrayObject(debugLines);
        //
        //// Draw the mesh to update the shadow map
        //graphicsEngine.drawLines(debugLines->getNumIndices());
        
        m_gameObjectManager->Render(uniformData);

        // Render the transparent objects after
        m_gameObjectManager->onTransparencyPass(uniformData);
        m_gameObjectManager->onSkyboxPass(uniformData);

        m_postProcessingFramebuffer->UnBind();
    }


    graphicsEngine.clear(glm::vec4(0, 0, 0, 1)); //clear the scene

    if (m_windowFrameBuffer != nullptr)
    {
        m_windowFrameBuffer->Bind();
    }

    // Post processing 
    m_postProcessingFramebuffer->PopulateShader(m_postProcessSSRQMaterial->GetShader());
    m_SSRQ->SetMaterial(m_postProcessSSRQMaterial);
    m_SSRQ->Render(uniformData, RenderingPath::Forward);

    if (m_windowFrameBuffer != nullptr)
    {
        m_windowFrameBuffer->UnBind();
    }

    // ui canvas of some sort with the ui camera
}

void Scene::onResize(Vector2 size)
{
    Resizable::onResize(size);
    GeometryBuffer::GetInstance().Resize(size);

    for (auto camera : m_gameObjectManager->getCameras())
    {
        camera->setScreenArea(size);
    }
    // resize the post processing frame buffer 
    m_postProcessingFramebuffer->onResize(size);

    if (m_windowFrameBuffer != nullptr)
    {
        m_windowFrameBuffer->onResize(size);
    }
}

LightManager* Scene::getLightManager()
{
    return m_lightManager.get();
}

GameObjectManager* Scene::getObjectManager()
{
    return m_gameObjectManager.get();
}

void Scene::onQuit()
{
    m_gameObjectManager->clearGameObjects();
}

void Scene::Save()
{
    m_gameObjectManager->saveGameObjectsToFile(m_filePath);
    Debug::Log("Scene saved to file path: " + m_filePath);
}

string Scene::GetFilePath()
{
    return m_filePath;
}

void Scene::SetWindowFrameBuffer(FramebufferPtr windowFrameBuffer)
{
    m_windowFrameBuffer = windowFrameBuffer;
}

Vector2 Scene::getFrameBufferSize()
{
    return Vector2(m_windowFrameBuffer->RenderTexture->getWidth(), m_windowFrameBuffer->RenderTexture->getHeight());
}

ImTextureID Scene::getRenderTexture()
{
    if (m_windowFrameBuffer->RenderTexture)
    {
        return (ImTextureID)m_windowFrameBuffer->RenderTexture->getId();
    }
    return ImTextureID();
}
