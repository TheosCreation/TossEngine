#pragma once
#include "Utils.h"

class TossEditor;

// AssetsBrowser.h
struct AssetItem {
    std::string uid;      // e.g. "Textures/ui/button.png"
    std::string name;     // "button.png"
    std::string type;     // "Texture2D"
    ResourcePtr   res;    // handle
};

struct AssetNode {
    std::string name;         // folder name
    std::string path;         // "Textures/ui"
    std::map<std::string, std::unique_ptr<AssetNode>> children;
    std::vector<AssetItem> items;
    bool open = false;
};

class AssetsBrowser {
public:
    AssetsBrowser(TossEditor* editor);
    ~AssetsBrowser();
    void Rebuild();
    void drawRightPane(AssetNode& node);
    void Draw();

private:
    TossEditor* Editor = nullptr;
    std::unique_ptr<AssetNode> m_root;
    std::string m_currentFolder;       // path relative to root, "" for root
    std::string m_renameUID;
    char m_renameBuf[256]{};

    char iDBuffer[256] = "";                            //!< Temporary input buffer for IDs.
    std::string selectedTypeName = "";                  //!< Selected resource type name.
    bool createResource = false;                        //!< True if resource creation is requested.
    
    ImGuiTextFilter m_filter;
    bool m_showGrid = false;
    float m_gridSize = 96.0f;

    void SelectResource(const ResourcePtr& r);
    void drawLeftTree(AssetNode& node);
    AssetNode* findNode(const std::string& path);
};
