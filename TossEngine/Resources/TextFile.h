/***
DeviousDevs
Auckland
New Zealand
(c) 2026 DeviousDevs
File Name : TextFile.h
Description : TextFile resource used to inspect plain text files from disk
Author : Theo Morris
Mail : theo.morris@outlook.co.nz
***/

#pragma once
#include "../Resource.h"
#include <string>

/**
 * @class TextFile
 * @brief Resource wrapper for text-based files that are read directly from disk.
 */
class TOSSENGINE_API TextFile : public Resource
{
public:
    TextFile(const std::string& uid, ResourceManager* mgr);

    void onCreateLate() override;
    void OnInspectorGUI() override;

    const std::string& GetContent() const
    {
        return m_content;
    }

private:
    std::string m_content;
};

REGISTER_RESOURCE(TextFile, ".txt", ".txt")

inline void to_json(json& j, TextFilePtr const& textFile)
{
    if (textFile)
    {
        j = json{ { "id", textFile->getUniqueID() } };
    }
    else
    {
        j = nullptr;
    }
}

inline void from_json(json const& j, TextFilePtr& textFile)
{
    if (j.contains("id"))
    {
        textFile = ResourceManager::GetInstance().get<TextFile>(j["id"].get<string>());
    }
}