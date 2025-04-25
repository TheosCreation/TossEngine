/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Shader.cpp
Description : Shader class is a wrapper class for handling shaders in OpenGL
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "Shader.h"
#include "Texture2D.h"
#include "TextureCubeMap.h"
#include "TossEngine.h"


Shader::Shader(const ShaderDesc& desc, const string& uniqueId, ResourceManager* manager) : Resource("", uniqueId, manager)
{
    m_programId = glCreateProgram();
    Attach(desc.vertexShaderFilePath, ShaderType::VertexShader);
    Attach(desc.fragmentShaderFilePath, ShaderType::FragmentShader);
    link();
    isLoaded = true;
}

Shader::Shader(const std::string& uid, ResourceManager* mgr) : Resource(uid, mgr)
{
    m_programId = glCreateProgram();
}

void Shader::onDestroy()
{
    if (isLoaded)
    {
        for (uint i = 0; i < 2; ++i) {
            if (m_attachedShaders[i] != 0) {
                glDetachShader(m_programId, m_attachedShaders[i]);
                glDeleteShader(m_attachedShaders[i]);
            }
        }
        isLoaded = false;
    }
    if (m_programId != 0) {
        glDeleteProgram(m_programId);
    }
}

void Shader::onCreateLate()
{
    if (m_vertexShaderFilePath.empty() || m_fragShaderFilePath.empty()) return;

    Attach(m_vertexShaderFilePath, ShaderType::VertexShader);
    Attach(m_fragShaderFilePath, ShaderType::FragmentShader);
    link();
    isLoaded = true;
}

void Shader::OnInspectorGUI()
{
    ImGui::Text(("Shader Inspector - ID: " + m_uniqueID).c_str());
    ImGui::Separator();

    bool pathsChanged = false;

    if (ImGui::BeginTable("ShaderPaths", 3,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg))
    {
        // --- Vertex row ---
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Vertex Shader:");
        ImGui::TableSetColumnIndex(1);
        if (m_vertexShaderFilePath.empty())
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Not Assigned");
        else
            ImGui::TextUnformatted(m_vertexShaderFilePath.c_str());
        ImGui::TableSetColumnIndex(2);
        if (ImGui::Button("Browse##Vert"))
        {
            auto chosen = TossEngine::GetInstance().openFileDialog("*.vert");
            if (!chosen.empty())
            {
                auto root = getProjectRoot();
                auto relPath = std::filesystem::relative(chosen, root).string();
                m_vertexShaderFilePath = relPath;
                pathsChanged = true;
            }
        }

        // --- Fragment row ---
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Fragment Shader:");
        ImGui::TableSetColumnIndex(1);
        if (m_fragShaderFilePath.empty())
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Not Assigned");
        else
            ImGui::TextUnformatted(m_fragShaderFilePath.c_str());
        ImGui::TableSetColumnIndex(2);
        if (ImGui::Button("Browse##Frag"))
        {
            auto chosen = TossEngine::GetInstance().openFileDialog("*.frag");
            if (!chosen.empty())
            {
                auto root = getProjectRoot();
                auto relPath = std::filesystem::relative(chosen, root).string();
                m_fragShaderFilePath = relPath;
                pathsChanged = true;
            }
        }

        ImGui::EndTable();
    }

    // if either path changed, and now both are set, recompile & relink
    if (pathsChanged &&
        !m_vertexShaderFilePath.empty() &&
        !m_fragShaderFilePath.empty())
    {
        // detach old shaders
        for (uint i = 0; i < 2; ++i)
        {
            if (m_attachedShaders[i] != 0)
            {
                glDetachShader(m_programId, m_attachedShaders[i]);
                glDeleteShader(m_attachedShaders[i]);
                m_attachedShaders[i] = 0;
            }
        }

        // attach & link anew
        Attach(m_vertexShaderFilePath, ShaderType::VertexShader);
        Attach(m_fragShaderFilePath, ShaderType::FragmentShader);
        link();

        isLoaded = true;
    }
}

bool Shader::Delete(bool deleteSelf)
{
    return false;
}

vector<UniformBinding> Shader::getBindings() const {
    vector<UniformBinding> bindings;

    // Query the number of active uniforms in the shader program.
    GLint uniformCount = 0;
    glGetProgramiv(m_programId, GL_ACTIVE_UNIFORMS, &uniformCount);

    // Temporary buffer for uniform names.
    char nameBuffer[256];

    // Loop over all active uniforms.
    for (int i = 0; i < uniformCount; i++) {
        GLint size = 0;
        GLenum type = 0;
        GLsizei length = 0;
        glGetActiveUniform(m_programId, i, sizeof(nameBuffer), &length, &size, &type, nameBuffer);

        // Create a UniformBinding instance.
        UniformBinding binding;
        binding.name = std::string(nameBuffer, length);
        binding.type = type;
        binding.size = size;

        // Add the binding to the MaterialDesc.
        bindings.push_back(binding);
    }

    return bindings;
}

void Shader::Attach(const std::string& filePath, const ShaderType& type)
{
    std::ifstream shaderStream;
    std::string shaderCode;

    // Open the shader file
    shaderStream.open(filePath.c_str(), std::ios::in);
    if (!shaderStream.good()) {
        Debug::LogWarning("Shader | Cannot read file: " + filePath);
        shaderStream.close();
        return;
    }

    // Read the shader code into a string
    shaderStream.seekg(0, std::ios::end);
    shaderCode.resize(static_cast<size_t>(shaderStream.tellg()));
    shaderStream.seekg(0, std::ios::beg);
    shaderStream.read(&shaderCode[0], shaderCode.size());
    shaderStream.close();

    // Preprocess the shader code to handle includes
    shaderCode = PreprocessShader(shaderCode);

    // Determine shader type for OpenGL
    GLenum glShaderType;
    if (type == ShaderType::VertexShader) {
        glShaderType = GL_VERTEX_SHADER;
    }
    else if (type == ShaderType::FragmentShader) {
        glShaderType = GL_FRAGMENT_SHADER;
    }
    else if (type == ShaderType::ComputeShader) {
        glShaderType = GL_COMPUTE_SHADER;
    }
    else {
        Debug::LogWarning("Shader | Unsupported shader type");
        return;
    }

    // Create and compile the shader
    uint shaderId = glCreateShader(glShaderType);
    const char* sourcePointer = shaderCode.c_str();
    glShaderSource(shaderId, 1, &sourcePointer, nullptr);
    glCompileShader(shaderId);

    // Check for compile errors
    int logLength = 0;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        std::vector<char> errorMessage(logLength + 1);
        glGetShaderInfoLog(shaderId, logLength, nullptr, &errorMessage[0]);
        Debug::LogError("Shader | " + filePath + " compiled with errors: " + std::string(errorMessage.data()), false);
        return;
    }

    // Attach the shader to the program
    glAttachShader(m_programId, shaderId);
    m_attachedShaders[static_cast<uint>(type)] = shaderId;

    Debug::Log("Shader | " + filePath + " compiled successfully");
}

// Function to read shader code from a file
std::string Shader::ReadShaderFile(const std::string& filePath) {
    std::ifstream shaderFile(filePath);
    std::stringstream shaderStream;

    if (shaderFile.is_open()) {
        shaderStream << shaderFile.rdbuf();
    }
    else {
        Debug::LogWarning("Shader | Could not open file: " + filePath);
    }

    return shaderStream.str();
}

// Function to preprocess shader code
std::string Shader::PreprocessShader(const std::string& shaderCode) {
    std::string processedCode;
    std::istringstream stream(shaderCode);
    std::string line;

    while (std::getline(stream, line)) {
        // Check for #include directive
        if (line.find("#include") != std::string::npos) {
            // Extract the file path from the line
            std::string includeFilePath = line.substr(line.find('"') + 1);
            includeFilePath = includeFilePath.substr(0, includeFilePath.find('"'));

            // Read the included file
            std::string includedCode = ReadShaderFile(includeFilePath);
            processedCode += includedCode + "\n"; // Add included code
        }
        else {
            processedCode += line + "\n"; // Add original line
        }
    }

    return processedCode;
}

void Shader::link() const
{
	glLinkProgram(m_programId);

	//get compile errors
	int logLength = 0;
	glGetShaderiv(m_programId, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		std::vector<char> errorMessage(logLength + 1);
		glGetShaderInfoLog(m_programId, logLength, NULL, &errorMessage[0]);
		Debug::LogWarning("Shader | " + errorMessage[0]);
		return;
	}
}

uint Shader::getId() const
{
	return m_programId;
}

void Shader::setTexture2D(const uint& textureId, uint slot, const std::string& bindingName) const
{
    auto glSlot = GL_TEXTURE0 + slot;
    glActiveTexture(glSlot); // activate the texture unit first before binding texture
    glBindTexture(GL_TEXTURE_2D, textureId);
    setInt(bindingName, slot);
}

void Shader::setTexture2D(const Texture2DPtr& texture, uint slot, const std::string& bindingName) const
{
    auto glSlot = GL_TEXTURE0 + slot;
    glActiveTexture(glSlot); // activate the texture unit first before binding texture

    if (texture != nullptr)
    {
        glBindTexture(GL_TEXTURE_2D, texture->getId());
        setInt(bindingName, slot);
    }
    else
    {
        //unbind the texture
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Shader::setTextureCubeMap(const TextureCubeMapPtr& texture, uint slot, const std::string& bindingName) const
{
    auto glSlot = GL_TEXTURE0 + slot;
    glActiveTexture(glSlot); // activate the texture unit first before binding/unbinding texture
    if (texture != nullptr)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture->getId());
        setInt(bindingName, slot);
    }
    else
    {
        //unbind the texture
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}