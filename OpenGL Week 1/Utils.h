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
#include <nlohmann\json.hpp>

using json = nlohmann::json; // will be using json to serialize classes and save entity objects and all

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
class InstancedMesh;
class SSRQuad;

// Type definitions for common engine variables
typedef unsigned int uint;
typedef glm::quat Quaternion;
typedef glm::mat4 Mat4; 
typedef glm::vec4 Vector4;
typedef glm::vec3 Vector3;
typedef glm::vec2 Vector2; 

// Type definitions for shared pointers
typedef std::shared_ptr<VertexArrayObject> VertexArrayObjectPtr;
typedef std::shared_ptr<Shader> ShaderPtr;
typedef std::shared_ptr<Resource> ResourcePtr;
typedef std::shared_ptr<Texture> TexturePtr;
typedef std::shared_ptr<Texture2D> Texture2DPtr;
typedef std::shared_ptr<ShadowMap> ShadowMapPtr;
typedef std::unique_ptr<Framebuffer> FramebufferPtr;
typedef std::shared_ptr<TextureCubeMap> TextureCubeMapPtr;
typedef std::shared_ptr<Mesh> MeshPtr;
typedef std::shared_ptr<InstancedMesh> InstancedMeshPtr;
typedef std::shared_ptr<HeightMap> HeightMapPtr;
typedef std::shared_ptr<SSRQuad> SSRQuadPtr;

// Using declarations to simplify the code and avoid typing the full namespace each time
using std::shared_ptr;
using std::unique_ptr;
using std::vector;
using std::string;

// Structure representing the transformation of an object in 3D space
struct Transform
{
    Vector3 position;       // Position of the object in world space (x, y, z)
    Quaternion rotation;    // Rotation of the object in world space represented as a quaternion
    Vector3 scale;          // Scale of the object in world space (x, y, z)

    // Default constructor initializes position to (0,0,0), rotation to no rotation, and scale to (1,1,1)
    Transform()
        : position(Vector3(0.0f, 0.0f, 0.0f)),                  // Initial position
        rotation(Quaternion(1.0f, 0.0f, 0.0f, 0.0f)),           // Initial rotation (identity)
        scale(Vector3(1.0f, 1.0f, 1.0f))                        // Initial scale
    {
    }

    // Returns the transformation matrix that combines translation, rotation, and scale
    Mat4 GetMatrix() const
    {
        // Create a translation matrix from the position
        Mat4 translationMatrix = glm::translate(Mat4(1.0f), position);
        // Create a rotation matrix from the quaternion
        Mat4 rotationMatrix = glm::toMat4(rotation);
        // Create a scale matrix from the scale vector
        Mat4 scaleMatrix = glm::scale(Mat4(1.0f), scale);

        // Combine the translation, rotation, and scale matrices
        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    // Sets a new position for the object
    void SetPosition(const Vector3& newPosition)
    {
        position = newPosition;
    }

    // Sets a new rotation for the object
    void SetRotation(const Quaternion& newRotation)
    {
        rotation = newRotation;
    }

    // Sets a new scale for the object
    void SetScale(const Vector3& newScale)
    {
        scale = newScale;
    }

    // Translates the object by a given vector
    void Translate(const Vector3& translation)
    {
        position += translation; // Update position by adding translation vector
    }

    // Rotates the object by a given quaternion
    void Rotate(const Quaternion& deltaRotation)
    {
        rotation = glm::normalize(deltaRotation * rotation); // Update rotation and normalize it
    }

    // Scales the object by a given scale factor
    void Scale(const Vector3& scaleFactor)
    {
        scale *= scaleFactor; // Update scale by multiplying with scale factor
    }

    // Returns the forward direction of the object based on its rotation
    Vector3 GetForward() const
    {
        return rotation * Vector3(0.0f, 0.0f, -1.0f); // Forward direction is along the z-axis
    }

    // Returns the right direction of the object based on its rotation
    Vector3 GetRight() const
    {
        return rotation * Vector3(1.0f, 0.0f, 0.0f); // Right direction is along the x-axis
    }

    // Returns the up direction of the object based on its rotation
    Vector3 GetUp() const
    {
        return rotation * Vector3(0.0f, 1.0f, 0.0f); // Up direction is along the y-axis
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
    string vertexShaderFileName;    // Filename of the vertex shader
    string fragmentShaderFileName;  // Filename of the fragment shader
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

// Enum representing camera types
enum class CameraType
{
    Orthogonal = 0, // Orthogonal camera
    Perspective     // Perspective camera
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
    KeyRight = 262,
    KeyLeft,
    KeyDown,
    KeyUp,
    KeyShift = 340,
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
};

// Struct representing a directional light
struct DirectionalLight
{
    Vector3 Direction;
    Vector3 Color;
    float SpecularStrength;
};

// Struct representing a point light
struct PointLight
{
    Vector3 Position;
    Vector3 Color;
    float SpecularStrength;

    float AttenuationConstant;
    float AttenuationLinear;
    float AttenuationExponent;
};

// Struct representing a spot light
struct SpotLight
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

// Normalizes a given vector using GLM library functions
// Template parameters:
// L      - The length of the vector (e.g., 2 for vec2, 3 for vec3)
// T      - The data type of the vector components (e.g., float, double)
// Q      - The qualifier for the vector (e.g., default, highp, lowp)
template<glm::length_t L, typename T, glm::qualifier Q>
GLM_FUNC_QUALIFIER glm::vec<L, T, Q> Normalize(glm::vec<L, T, Q> const& vector)
{
    return glm::normalize(vector); // Returns the normalized vector
}

// Converts a value of any type to a string
// Template parameter:
// T      - The type of the value to be converted
template <typename T>
std::string ToString(const T& value) {
    return std::to_string(value); // Uses std::to_string to convert the value to a string
}

class Debug
{
public:
    // Static methods for logging messages
    static void Log(const string& message) {
        PrintMessage(message, "Log");
    };
    static void LogError(const string& message) {
        PrintMessage(message, "Error");
        throw std::runtime_error(message);
    }
    static void LogWarning(const string& message) {
        PrintMessage(message, "Warning");
    }

private:
    // Helper methods to format and print messages
    static void PrintMessage(const string& message, const string& type) {
        printf("[%s] %s\n", type.c_str(), message.c_str());
    }
};

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