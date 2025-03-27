/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Utils.h
Description : header file that represents a utility header to structure this project to make it more readable
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <vector>
#include <map>
#include <glm.hpp>
#include "Rect.h"
#include "Math.h"
#include <unordered_map>
#include <any>
#include <functional>
#include <algorithm>
#include <typeinfo>
#include <typeindex>
#include <fstream>
#include <filesystem>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <nlohmann\json.hpp>
#include <glew.h>
#include "Debug.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Quaternion.h"
#include "Mat4.h"
#include "Mat3.h"


using json = nlohmann::json; // will be using json to serialize classes and save GameObject objects and all
namespace fs = std::filesystem;

// Forward declarations of classes
class UniformBuffer;
class VertexArrayObject;
class Shader;
class Texture2D;
class ShadowMap;
class Framebuffer;
class TextureCubeMap;
class ResourceManager;
class Resource;
class Texture;
class HeightMap;
class Mesh;
struct ProjectSettings;
class Sound;
class Component;
class Material;
class Scene;
class GameObject;
class TossPlayerSettings;

// Type definitions for common engine variables
typedef unsigned int uint;

// Type definitions for shared pointers
typedef std::shared_ptr<VertexArrayObject> VertexArrayObjectPtr;
typedef std::shared_ptr<Shader> ShaderPtr;
typedef std::shared_ptr<Resource> ResourcePtr;
typedef std::shared_ptr<Texture> TexturePtr;
typedef std::shared_ptr<Texture2D> Texture2DPtr;
typedef std::shared_ptr<ShadowMap> ShadowMapPtr;
typedef std::shared_ptr<Framebuffer> FramebufferPtr;
typedef std::shared_ptr<TextureCubeMap> TextureCubeMapPtr;
typedef std::shared_ptr<Mesh> MeshPtr;
typedef std::shared_ptr<HeightMap> HeightMapPtr;
typedef std::unique_ptr<ProjectSettings> ProjectSettingsPtr;
typedef std::unique_ptr<TossPlayerSettings> TossPlayerSettingsPtr;
typedef std::shared_ptr<Sound> SoundPtr;
typedef std::shared_ptr<Material> MaterialPtr;
typedef std::shared_ptr<Scene> ScenePtr;

// Using declarations to simplify the code and avoid typing the full namespace each time
using std::shared_ptr;
using std::unique_ptr;
using std::vector;
using std::string;

// Structure representing the transformation of an object in 3D space
struct TOSSENGINE_API Transform
{
    Vector3 position;         // World position
    Quaternion rotation;      // World rotation
    Vector3 scale;            // World scale

    Vector3 localPosition;    // Local position relative to parent
    Quaternion localRotation; // Local rotation relative to parent
    Vector3 localScale;       // Local scale relative to parent

    Transform* parent;                // Pointer to parent transform (nullptr if root)
    std::vector<Transform*> children; // List of pointers to child transforms

    GameObject* gameObject;   // Pointer to the GameObject this Transform is attached to

    // Default Constructor for none attached transforms and just data
    Transform()
        : position(Vector3(0.0f, 0.0f, 0.0f)),
        rotation(Quaternion(1.0f, 0.0f, 0.0f, 0.0f)),
        scale(Vector3(1.0f, 1.0f, 1.0f)),
        localPosition(Vector3(0.0f, 0.0f, 0.0f)),
        localRotation(Quaternion(1.0f, 0.0f, 0.0f, 0.0f)),
        localScale(Vector3(1.0f, 1.0f, 1.0f)),
        parent(nullptr),
        gameObject(nullptr)
    {
    }

    // Constructor initializes defaults and attaches GameObject
    Transform(GameObject* attachedGameObject)
        : position(Vector3(0.0f, 0.0f, 0.0f)),
        rotation(Quaternion(1.0f, 0.0f, 0.0f, 0.0f)),
        scale(Vector3(1.0f, 1.0f, 1.0f)),
        localPosition(Vector3(0.0f, 0.0f, 0.0f)),
        localRotation(Quaternion(1.0f, 0.0f, 0.0f, 0.0f)),
        localScale(Vector3(1.0f, 1.0f, 1.0f)),
        parent(nullptr),
        gameObject(attachedGameObject)
    {
    }

    Mat4 GetMatrix() const
    {
        Mat4 translationMatrix = position.ToTranslation();
        Mat4 rotationMatrix = rotation.ToMat4();
        Mat4 scaleMatrix = scale.ToScale();

        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    void SetMatrix(const Mat4& matrix)
    {
        const glm::mat4& raw = matrix.value;

        // Extract translation (last column)
        position = Vector3(raw[3]);

        // Extract scale
        scale.x = Vector3(raw[0]).Length();
        scale.y = Vector3(raw[1]).Length();
        scale.z = Vector3(raw[2]).Length();

        // Remove scale from matrix to isolate rotation
        glm::mat4 rotMatrix = glm::mat4(1.0f);
        rotMatrix[0] = glm::vec4(glm::normalize(glm::vec3(raw[0])), 0.0f);
        rotMatrix[1] = glm::vec4(glm::normalize(glm::vec3(raw[1])), 0.0f);
        rotMatrix[2] = glm::vec4(glm::normalize(glm::vec3(raw[2])), 0.0f);
        rotMatrix[3] = glm::vec4(0, 0, 0, 1);

        rotation = Quaternion(glm::quat_cast(rotMatrix));
    }

    void UpdateWorldTransform()
    {
        if (parent)
        {
            position = parent->position + (parent->rotation * (parent->scale * localPosition));
            rotation = parent->rotation * localRotation;
            rotation.Normalize();
            scale = parent->scale * localScale;
        }

        // Recursively update children
        for (Transform* child : children)
        {
            child->UpdateWorldTransform();
        }
    }

    Vector3 ToEulerAngles()
    {
        return rotation.ToEulerAngles();
    }

    void SetPosition(const Vector3& newPosition)
    {
        position = newPosition;
    }

    void SetRotation(const Quaternion& newRotation)
    {
        rotation = newRotation;
    }

    void SetScale(const Vector3& newScale)
    {
        scale = newScale;
    }

    void Translate(const Vector3& translation)
    {
        position += translation;
    }

    void Rotate(const Quaternion& deltaRotation)
    {
        rotation = deltaRotation * rotation;
        rotation.Normalize();
    }

    void Scale(const Vector3& scaleFactor)
    {
        scale *= scaleFactor;
    }

    Vector3 GetForward() const
    {
        return rotation * Vector3(0.0f, 0.0f, -1.0f);
    }

    Vector3 GetRight() const
    {
        return rotation * Vector3(1.0f, 0.0f, 0.0f);
    }

    Vector3 GetUp() const
    {
        return rotation * Vector3(0.0f, 1.0f, 0.0f);
    }

    void SetParent(Transform* newParent)
    {
        if (parent)
        {
            parent->RemoveChild(this);
        }

        parent = newParent;
        if (parent)
        {
            parent->children.push_back(this);
        }
    }

private:
    void RemoveChild(Transform* child)
    {
        children.erase(std::remove(children.begin(), children.end(), child), children.end());
    }
};

namespace QuaternionUtils
{
    // Inline function to create a quaternion that looks in the specified direction
    inline Quaternion LookAt(const glm::vec3& direction, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f))
    {
        // Ensure the direction is normalized
        glm::vec3 forward = glm::normalize(direction);

        // Compute the right vector
        glm::vec3 right = glm::normalize(glm::cross(up, forward));

        // Recalculate the up vector to make sure it is orthogonal
        glm::vec3 recalculatedUp = glm::cross(forward, right);

        // Create a 3x3 rotation matrix
        glm::mat3 rotationMatrix(right, recalculatedUp, forward);

        // Convert the matrix to a quaternion
        return glm::quat_cast(rotationMatrix);
    }
}

inline glm::mat4 LookAt(const Vector3& eye, const Vector3& center, const Vector3& up) {
    return glm::lookAt(static_cast<glm::vec3>(eye),
        static_cast<glm::vec3>(center),
        static_cast<glm::vec3>(up));
}


// Structure representing a vertex in 3D space
struct Vertex
{
    Vector3 position;   // The 3D position of the vertex (x, y, z)
    Vector2 texCoords;  // The texture coordinates of the vertex (u, v) for texture mapping
    Vector3 normal;     // The normal vector at the vertex used for lighting calculations (x, y, z)
};

// Struct representing a vertex attribute
struct VertexAttribute
{
    uint numElements = 0; // Number of elements in the vertex attribute
};

// Struct representing a vertex buffer description
struct VertexBufferDesc
{
    void* verticesList = nullptr;               // Pointer to the list of vertices
    uint vertexSize = 0;                        // Size of a single vertex
    uint listSize = 0;                          // Size of the vertex list

    VertexAttribute* attributesList = nullptr;  // Pointer to the list of vertex attributes
    uint attributesListSize = 0;                // Size of the vertex attributes list
};

// Struct representing an index buffer description
struct IndexBufferDesc
{
    void* indicesList = nullptr;    // Pointer to the list of indices
    uint listSize = 0;              // Size of the index list
};

// Struct representing a shader description
struct ShaderDesc
{
    string vertexShaderFilePath;    // Filename of the vertex shader
    string fragmentShaderFilePath;  // Filename of the fragment shader
};

// Struct representing a uniform buffer description
struct UniformBufferDesc
{
    uint size = 0; // Size of the uniform buffer
};

// Struct representing uniform data
struct UniformData
{
    Mat4 viewMatrix;            // View matrix
    Mat4 projectionMatrix;      // Projection matrix
    Mat4 uiViewMatrix;          // UI view matrix
    Mat4 uiProjectionMatrix;    // UI projection matrix
    float currentTime;          // Current time
    Vector3 cameraPosition;     // Camera Position
};

struct NewExtraTextureData {
    // Map from string to a tuple of TexturePtr and uint
    std::unordered_map<std::string, std::tuple<TexturePtr, uint>> textureMap;

    // Add a texture with a string key
    void AddTexture(const std::string& key, TexturePtr texture, uint value) {
        textureMap[key] = std::make_tuple(texture, value);
    }
};

// Structure to manage uniform data using a map
struct NewUniformData {
    // Map to store uniform data with string keys and values of any type
    std::unordered_map<std::string, std::any> dataMap;

    // Method to create and store a new uniform data entry
    template<typename T>
    void CreateData(const std::string& name, const T& value) {
        dataMap[name] = value; // Store the value in the map with the given name as the key
    }

    // Method to retrieve uniform data by name, casting it to the specified type
    template<typename T>
    T GetData(const std::string& name) const {
        return std::any_cast<T>(dataMap.at(name)); // Access and cast the value from the map to the specified type
    }
};

// Struct representing a texture 2D description
struct Texture2DDesc
{
    unsigned char* textureData = nullptr;   // Pointer to the texture data
    Rect textureSize = {};                  // Size of the texture
    uint numChannels = 0;                   // Number of channels in the texture

};

// Structure to hold information about a heightmap
struct HeightMapInfo {
    std::string filePath = ""; // The file path to the heightmap texture or data file
    uint width = 0;            // The width of the heightmap in number of cells
    uint depth = 0;            // The depth (height) of the heightmap in number of cells
    float cellSpacing = 1.0f;  // The spacing between each cell in the heightmap, typically in world units
};

// Struct representing a height map description
struct HeightMapDesc
{
    std::vector<float> data;
};

// Struct representing a texture Cubemap description
struct TextureCubeMapDesc
{
    std::vector<void*> textureData; // Pointers to the texture data for each face
    Rect textureSize = {};          // Size of each texture face
    uint numChannels = 0;           // Number of channels in each texture face
};

// Struct representing a sound description
struct SoundDesc
{
    bool is3D = false;
    bool isLoop = false;
    float volume = 1.0f;
    float reverbAmount = 0.0f;
};

struct UniformBinding {
    std::string name;  // Name of the uniform
    GLenum type;       // GL type (e.g. GL_FLOAT_VEC3, GL_INT, etc.)
    GLint size;
};

struct MaterialDesc
{
    std::vector<UniformBinding> uniformBindings;
};

// Enum representing camera types
enum class CameraType
{
    Orthogonal = 0, // Orthogonal camera
    Perspective     // Perspective camera
};

enum class RenderingPath
{
    Deferred = 0,   // Deferred Rendering
    Forward,         // Forward Rendering
    Unknown
};

// Enum representing triangle types
enum class TriangleType
{
    TriangleList = 0,   // Triangle list
    TriangleStrip,      // Triangle strip
    Points              // Points
};

// Enum representing cull types
enum class CullType
{
    BackFace = 0,   // Cull back face
    FrontFace,      // Cull front face
    Both,           // Cull both faces
    None            // Cull no faces
};

// Enum representing blend types used in rendering
enum class BlendType
{
    Zero,               // (0, 0, 0, 0) - The color is multiplied by zero
    One,                // (1, 1, 1, 1) - The color is multiplied by one (no change)
    SrcColor,           // Source color - Multiplies the destination color by the source color
    OneMinusSrcColor,   // One minus source color - Multiplies the destination color by (1 - source color)
    DstColor,           // Destination color - Multiplies the source color by the destination color
    OneMinusDstColor,   // One minus destination color - Multiplies the source color by (1 - destination color)
    SrcAlpha,           // Source alpha - Multiplies the destination color by the source alpha
    OneMinusSrcAlpha,   // One minus source alpha - Multiplies the destination color by (1 - source alpha)
    DstAlpha,           // Destination alpha - Multiplies the source color by the destination alpha
    OneMinusDstAlpha,   // One minus destination alpha - Multiplies the source color by (1 - destination alpha)
    ConstantColor,      // A constant color specified in the blend state
    OneMinusConstantColor, // One minus the constant color
    ConstantAlpha,      // A constant alpha value specified in the blend state
    OneMinusConstantAlpha // One minus the constant alpha
};

// Enum representing different texture types
enum class TextureType
{
    Default,    // Standard texture used for general purposes
    Heightmap   // Special texture type used for height mapping in terrain generation
};

// Enum representing depth test functions
enum class DepthType
{
    Never,          // Never passes the depth test
    Less,           // Passes if the incoming depth is less than the stored depth
    Equal,          // Passes if the incoming depth equals the stored depth
    LessEqual,      // Passes if the incoming depth is less than or equal to the stored depth
    Greater,        // Passes if the incoming depth is greater than the stored depth
    NotEqual,       // Passes if the incoming depth is not equal to the stored depth
    GreaterEqual,   // Passes if the incoming depth is greater than or equal to the stored depth
    Always          // Always passes the depth test
};

// Enum representing stencil buffer operations
enum class StencilOperationType
{
    Set,            // Sets the stencil buffer to a reference value
    ResetNotEqual,  // Resets the stencil value if it is not equal to the reference value
    ResetAlways     // Always resets the stencil value
};

// Enum representing winding orders
enum class WindingOrder
{
    ClockWise = 0,      // Clockwise winding order
    CounterClockWise    // Counter-clockwise winding order
};

// Enum representing shader types
enum class ShaderType
{
    VertexShader = 0,   // Vertex shader
    FragmentShader,     // Fragment shader
    ComputeShader       // Compute shader
};

// Enum representing key codes
enum Key
{
    Key0 = 48,
    Key1,
    Key2,
    Key3,
    Key4,
    Key5,
    Key6,
    Key7,
    Key8,
    Key9,
    KeyA = 65,
    KeyB,
    KeyC,
    KeyD,
    KeyE,
    KeyF,
    KeyG,
    KeyH,
    KeyI,
    KeyJ,
    KeyK,
    KeyL,
    KeyM,
    KeyN,
    KeyO,
    KeyP,
    KeyQ,
    KeyR,
    KeyS,
    KeyT,
    KeyU,
    KeyV,
    KeyW,
    KeyX,
    KeyY,
    KeyZ, 
    KeyTab = 258,
    KeyF1 = 290,
    KeyF2,
    KeyF3,
    KeyF4,
    KeyF5,
    KeyF6,
    KeyF7,
    KeyF8,
    KeyF9,
    KeyF10,
    KeyF11,
    KeyF12,
    KeyEscape = 256,
    KeyDelete = 261,
    KeyRight = 262,
    KeyLeft,
    KeyDown,
    KeyUp,
    KeyShift = 340,
    KeyLeftControl = 341,
};

// Enum representing mouse buttons
enum MouseButton
{
    MouseButtonLeft,    // Left mouse button
    MouseButtonRight,   // Right mouse button
    MouseButtonMiddle,  // Middle mouse button
};

// Struct of predefined color constants.
struct Color {
    static constexpr Vector3 Red = Vector3(1.0f, 0.0f, 0.0f);
    static constexpr Vector3 Green = Vector3(0.0f, 1.0f, 0.0f);
    static constexpr Vector3 Blue = Vector3(0.0f, 0.0f, 1.0f);
    static constexpr Vector3 White = Vector3(1.0f, 1.0f, 1.0f);
    static constexpr Vector3 Black = Vector3(0.0f, 0.0f, 0.0f);
    static constexpr Vector3 Gray = Vector3(0.5f, 0.5f, 0.5f);
    static constexpr Vector3 Purple = Vector3(1.0f, 0.0f, 1.0f);
};

// Struct representing a directional light
struct DirectionalLightData
{
    Vector3 Direction;
    Vector3 Color;
    float SpecularStrength;
};

// Struct representing a point light
struct PointLightData
{
    Vector3 Position;
    Vector3 Color;
    float SpecularStrength;

    float AttenuationConstant;
    float AttenuationLinear;
    float AttenuationExponent;

    float Radius;
};

// Struct representing a spot light
struct SpotLightData
{
    Vector3 Position;
    Vector3 Direction;
    Vector3 Color;
    float SpecularStrength;
    float CutOff;
    float OuterCutOff;

    float AttenuationConstant;
    float AttenuationLinear;
    float AttenuationExponent;
};

// Converts a value of any type to a string
// Template parameter:
// T      - The type of the value to be converted
template <typename T>
std::string ToString(const T& value) {
    return std::to_string(value); // Uses std::to_string to convert the value to a string
}

// Specialization of ToString for RenderingPath
template <>
inline std::string ToString<RenderingPath>(const RenderingPath& value) 
{
    switch (value)
    {
    case RenderingPath::Deferred: return "Deferred Rendering";
    case RenderingPath::Forward: return "Forward Rendering";
    default: return "Unknown";
    }
}

// Specialization of ToString for CameraType
template <>
inline std::string ToString<CameraType>(const CameraType& value)
{
    switch (value)
    {
    case CameraType::Perspective: return "Perspective";
    case CameraType::Orthogonal: return "Orthogonal";
    default: return "Unknown";
    }
}

template <typename T>
inline T FromString(const std::string& input)
{
    std::stringstream ss(input);
    T value;

    // Handle boolean values
    if constexpr (std::is_same_v<T, bool>) {
        if (input == "true") return true;
        if (input == "false") return false;
        return static_cast<T>(std::stoi(input) != 0); // Handle "1"/"0"
    }
    // Handle basic types like int, float, double
    else {
        ss >> value;
    }

    return value;
}

// Specialization of FromString for RenderingPath
template <>
inline RenderingPath FromString<RenderingPath>(const std::string& input)
{
    static const std::unordered_map<std::string, RenderingPath> enumMap = {
        {"Forward Rendering", RenderingPath::Forward},
        {"Deferred Rendering", RenderingPath::Deferred}
    };

    auto it = enumMap.find(input);
    return (it != enumMap.end()) ? it->second : RenderingPath::Unknown;
}

// Specialization of FromString for CameraType
template <>
inline CameraType FromString<CameraType>(const std::string& input)
{
    static const std::unordered_map<std::string, CameraType> enumMap = {
        {"Perspective", CameraType::Perspective},
        {"Orthogonal", CameraType::Orthogonal}
    };

    auto it = enumMap.find(input);
    return (it != enumMap.end()) ? it->second : CameraType::Perspective;
}

inline std::string FindSolutionPath() {
    std::filesystem::path currentPath = std::filesystem::current_path();

    while (true) {
        for (const auto& entry : std::filesystem::directory_iterator(currentPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".sln") {
                return entry.path().string(); // Return first .sln file found
            }
        }

        std::filesystem::path parentPath = currentPath.parent_path();
        if (parentPath == currentPath) {
            break; // Reached root
        }

        currentPath = parentPath;
    }

    return ""; // No .sln found
}

inline std::string getProjectRoot()
{
    static std::string cachedRoot = [] {
        std::filesystem::path root = std::filesystem::current_path();
        return root.string();
        }();
    return cachedRoot;
}

inline string getMSBuildPath() {
    const char* vswhereCmd =
        R"("C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe)";

    std::array<char, 512> buffer;
    std::string result;

    // Use pipe to execute command and get output
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(vswhereCmd, "r"), _pclose);
    if (!pipe) {
        throw std::runtime_error("Failed to run vswhere.");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    // Remove potential trailing newline
    result.erase(result.find_last_not_of(" \n\r\t") + 1);
    return result;
}



// Helper function to clean up the class name
inline std::string getClassName(const std::type_info& typeInfo)
{
    std::string name = typeInfo.name();

    // Clean up the name (compiler-specific)
#ifdef _MSC_VER
    // MSVC returns "class Player", so we remove the "class " prefix
    size_t pos = name.find("class ");
    if (pos != std::string::npos)
    {
        name.erase(pos, 6); // Remove "class "
    }
#else
    // GCC/Clang returns mangled names like "6Player", so we demangle them
    int status = 0;
    char* demangled = abi::__cxa_demangle(name.c_str(), nullptr, nullptr, &status);
    if (status == 0)
    {
        name = demangled;
        free(demangled);
    }
#endif

    return name;
}