/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Shader.h
Description : Shader class is a wrapper class for handling shaders in OpenGL
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once
#include <glew.h>
#include <glm.hpp>
#include "Utils.h"

/**
 * @class Shader
 * @brief A wrapper class for handling shaders in OpenGL.
 */
class Shader
{
public:
    /**
     * @brief Constructor for the standard Shader class.
     * @param desc A description of the shader.
     */
    Shader(const ShaderDesc& desc);

    /**
     * @brief Constructor for the compute Shader class.
     * @param computeFileName File name of the compute shader.
     */
    Shader(const string computeFileName);

    /**
     * @brief Destructor for the Shader class.
     */
    ~Shader();

    /**
     * @brief Gets the ID of the shader program.
     * @return The ID of the shader program.
     */
    uint getId();

    /**
     * @brief Sets the active 2D texture by texture ID.
     * @param textureId The ID of the texture to bind.
     * @param slot The texture slot to bind the texture to.
     * @param bindingName The name of the binding in the shader.
     */
    void setTexture2D(const uint textureId, uint slot, std::string bindingName);

    /**
     * @brief Sets the active 2D texture using a shared pointer to Texture2D.
     * @param texture A shared pointer to the Texture2D to set.
     * @param slot The texture slot to bind the texture to.
     * @param bindingName The name of the binding in the shader.
     */
    void setTexture2D(const TexturePtr& texture, uint slot, std::string bindingName);

    /**
     * @brief Sets the active Cube Map texture using a shared pointer.
     * @param texture A shared pointer to the Texture2D to set.
     * @param slot The texture slot to bind the texture to.
     * @param bindingName The name of the binding in the shader.
     */
    void setTextureCubeMap(const TexturePtr& texture, uint slot, std::string bindingName);

    /**
     * @brief Sends a mat4 into the shader.
     * @param name The name of the uniform variable in the shader.
     * @param mat The mat4 value to send.
     */
    void setMat4(const std::string& name, const Mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(m_programId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    /**
     * @brief Sends an array of mat4 values into the shader.
     * @param name The name of the uniform variable in the shader.
     * @param mat The array of mat4 values to send.
     * @param count The number of mat4 values in the array.
     */
    void setMat4Array(const std::string& name, const Mat4 mat[], int count) const
    {
        glUniformMatrix4fv(glGetUniformLocation(m_programId, name.c_str()), count, GL_FALSE, &mat[0][0][0]);
    }

    /**
     * @brief Sends a float value into the shader.
     * @param name The name of the uniform variable in the shader.
     * @param value The float value to send.
     */
    void setFloat(const std::string& name, const float value) const
    {
        glUniform1f(glGetUniformLocation(m_programId, name.c_str()), value);
    }

    /**
     * @brief Sends a vec3 into the shader.
     * @param name The name of the uniform variable in the shader.
     * @param value The vec3 value to send.
     */
    void setVec3(const std::string& name, const Vector3& value) const
    {
        glUniform3fv(glGetUniformLocation(m_programId, name.c_str()), 1, &value[0]);
    }
    
    /**
    * @brief Sends a bool into the shader.
    * @param name The name of the uniform variable in the shader.
    * @param value The bool value to send.
    */
    void setBool(const std::string& name, const bool& value) const
    {
        glUniform1i(glGetUniformLocation(m_programId, name.c_str()), static_cast<int>(value));
    }

    /**
     * @brief Sends a vec4 into the shader.
     * @param name The name of the uniform variable in the shader.
     * @param value The vec4 value to send.
     */
    void setVec4(const std::string& name, const Vector4& value) const
    {
        glUniform4fv(glGetUniformLocation(m_programId, name.c_str()), 1, &value[0]);
    }

    /**
     * @brief Sends a vec2 into the shader.
     * @param name The name of the uniform variable in the shader.
     * @param value The vec2 value to send.
     */
    void setVec2(const std::string& name, const Vector2& value) const
    {
        glUniform2fv(glGetUniformLocation(m_programId, name.c_str()), 1, &value[0]);
    }

    /**
     * @brief Sends an int into the shader.
     * @param name The name of the uniform variable in the shader.
     * @param value The int value to send.
     */
    void setInt(const std::string& name, const int value) const
    {
        glUniform1i(glGetUniformLocation(m_programId, name.c_str()), value);
    }

    /**
     * @brief Sends an unsigned int into the shader.
     * @param name The name of the uniform variable in the shader.
     * @param value The unsigned int value to send.
     */
    void setUint(const std::string& name, const uint value) const
    {
        glUniform1ui(glGetUniformLocation(m_programId, name.c_str()), value);
    }

private:
    /**
     * @brief Attaches a shader to the program.
     * @param filename The filename of the shader source.
     * @param type The type of the shader (vertex, fragment, etc.).
     */
    void Attach(const std::string& filename, const ShaderType& type);

    /**
     * @brief Reads the shader source file.
     * @param filePath The path to the shader file.
     * @return The contents of the shader file as a string.
     */
    std::string ReadShaderFile(const std::string& filePath);

    /**
     * @brief Preprocesses the shader code.
     * @param shaderCode The original shader code to preprocess.
     * @return The preprocessed shader code.
     */
    std::string PreprocessShader(const std::string& shaderCode);

    /**
     * @brief Links the shader program.
     */
    void link();

private:
    uint m_programId = 0; // The ID of the shader program.
    uint m_attachedShaders[2] = {}; // The IDs of the attached shaders.
};