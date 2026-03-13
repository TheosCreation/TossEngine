#include "AssetBrowser.h"

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")
#endif

#include <ObjectArray.h>
#include "ResourceManager.h"
#include "Resource.h"
#include "GameObject.h"
#include "Prefab.h"
#include <cstring>

#include "TossEditor.h"

// AssetsBrowser.cpp
static std::vector<std::string> SplitPath(const std::string& p) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : p) {
        if (c == '/' || c == '\\') { if (!cur.empty()) { out.push_back(cur); cur.clear(); } }
        else cur.push_back(c);
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

static void EnsureFolderNode(AssetNode& rootNode, const std::string& relFolder)
{
    if (relFolder.empty())
    {
        return;
    }

    std::vector<std::string> parts = SplitPath(relFolder);
    AssetNode* node = &rootNode;

    for (size_t i = 0; i < parts.size(); ++i)
    {
        const std::string& seg = parts[i];
        std::unique_ptr<AssetNode>& childPtr = node->children[seg];

        if (!childPtr)
        {
            std::unique_ptr<AssetNode> newNode = std::make_unique<AssetNode>();
            newNode->name = seg;
            if (node->path.empty())
            {
                newNode->path = seg;
            }
            else
            {
                newNode->path = node->path + "/" + seg;
            }

            childPtr = std::move(newNode);
        }

        node = childPtr.get();
    }
}

static std::string JoinPath(const std::string& a, const std::string& b)
{
    if (a.empty())
    {
        return b;
    }
    return a + "/" + b;
}

static std::string GetAssetsFolderPathFromCurrentFolder(const std::string& currentFolder)
{
    if (currentFolder.empty())
    {
        return "Assets"; // Default to Assets if at root
    }
    
    // If currentFolder already starts with Assets or Internal, just return it
    if (currentFolder.find("Assets") == 0 || currentFolder.find("Internal") == 0)
    {
        return currentFolder;
    }
    
    // Otherwise assume Assets
    return JoinPath("Assets", currentFolder);
}

static std::string BuildResourceAssetPath(const std::string& currentFolder, const std::string& typeName, const std::string& resourceId)
{
    const std::string folderPath = GetAssetsFolderPathFromCurrentFolder(currentFolder);
    const std::string extension = ResourceManager::GetInstance().GetExtensionForType(typeName);
    const std::string fileName = resourceId + extension;
    return JoinPath(folderPath, fileName);
}

static std::string GetFileNameFromPath(const std::string& path)
{
    std::filesystem::path p(path);
    return p.filename().string();
}

static std::string GetRelativePathFromAssets(const std::string& fullPath)
{
    std::filesystem::path full(fullPath);
    
    // Check if it's in Assets folder
    if (fullPath.find("Assets") == 0 || fullPath.find("Assets/") == 0 || fullPath.find("Assets\\") == 0)
    {
        std::filesystem::path assets("Assets");
        std::filesystem::path rel = full.lexically_relative(assets);
        return "Assets/" + rel.generic_string();
    }
    
    // Check if it's in Internal folder
    if (fullPath.find("Internal") == 0 || fullPath.find("Internal/") == 0 || fullPath.find("Internal\\") == 0)
    {
        std::filesystem::path internal("Internal");
        std::filesystem::path rel = full.lexically_relative(internal);
        return "Internal/" + rel.generic_string();
    }
    
    return fullPath;
}

static std::string GetParentFolderFromPath(const std::string& relativePath)
{
    std::filesystem::path p(relativePath);
    if (p.has_parent_path())
    {
        return p.parent_path().generic_string();
    }
    return "";
}

AssetsBrowser::AssetsBrowser(TossEditor* editor)
{
    Editor = editor;
}

AssetsBrowser::~AssetsBrowser()
{
}


void AssetsBrowser::Rebuild()
{
    m_root = std::make_unique<AssetNode>();
    m_root->name = "Root";
    m_root->path = "";

    // Build folder structure for both Assets and Internal
    std::vector<std::string> rootFolders = {"Assets", "Internal"};
    
    for (const auto& rootFolderName : rootFolders)
    {
        const std::filesystem::path folderRoot = std::filesystem::path(rootFolderName);
        
        // Create the root folder node (Assets or Internal)
        std::unique_ptr<AssetNode>& rootFolderNode = m_root->children[rootFolderName];
        if (!rootFolderNode)
        {
            rootFolderNode = std::make_unique<AssetNode>();
            rootFolderNode->name = rootFolderName;
            rootFolderNode->path = rootFolderName;
        }
        
        if (std::filesystem::exists(folderRoot))
        {
            std::filesystem::recursive_directory_iterator endIt;
            std::filesystem::recursive_directory_iterator it(folderRoot, std::filesystem::directory_options::skip_permission_denied);

            for (; it != endIt; ++it)
            {
                if (!it->is_directory())
                {
                    continue;
                }

                std::filesystem::path dirPath = it->path();
                std::filesystem::path relPath = dirPath.lexically_relative(folderRoot);
                std::string relFolder = relPath.generic_string();

                // Navigate from root folder node
                AssetNode* currentNode = rootFolderNode.get();
                std::vector<std::string> parts = SplitPath(relFolder);
                
                for (const auto& part : parts)
                {
                    std::unique_ptr<AssetNode>& childPtr = currentNode->children[part];
                    if (!childPtr)
                    {
                        std::unique_ptr<AssetNode> newNode = std::make_unique<AssetNode>();
                        newNode->name = part;
                        if (currentNode->path.empty())
                        {
                            newNode->path = part;
                        }
                        else
                        {
                            newNode->path = currentNode->path + "/" + part;
                        }
                        childPtr = std::move(newNode);
                    }
                    currentNode = childPtr.get();
                }
            }
        }
    }

    // Now layer imported/known resources into the folder nodes
    ResourceManager& rm = ResourceManager::GetInstance();
    std::map<std::string, ResourcePtr>& map = rm.GetAllResources();

    for (auto& kv : map)
    {
        const std::string& uid = kv.first;
        const ResourcePtr& res = kv.second;

        if (!res)
        {
            continue;
        }

        std::string assetPath = res->getPath();
        
        // Skip resources without a valid path
        if (assetPath.empty())
        {
            continue;
        }

        // Determine which root folder this belongs to
        std::string rootFolder;
        std::string relativePath;
        
        if (assetPath.find("Assets/") == 0 || assetPath.find("Assets\\") == 0)
        {
            rootFolder = "Assets";
            std::filesystem::path full(assetPath);
            std::filesystem::path rel = full.lexically_relative(std::filesystem::path("Assets"));
            relativePath = rel.generic_string();
        }
        else if (assetPath.find("Internal/") == 0 || assetPath.find("Internal\\") == 0)
        {
            rootFolder = "Internal";
            std::filesystem::path full(assetPath);
            std::filesystem::path rel = full.lexically_relative(std::filesystem::path("Internal"));
            relativePath = rel.generic_string();
        }
        else
        {
            // Skip resources that are not in Assets or Internal
            continue;
        }
        
        // Get the parent folder path
        std::string parentFolder = GetParentFolderFromPath(relativePath);
        
        // Start from the appropriate root folder node
        AssetNode* node = m_root->children[rootFolder].get();
        if (!node)
        {
            continue;
        }
        
        // Navigate to the parent folder node
        std::vector<std::string> parts = SplitPath(parentFolder);
        for (size_t i = 0; i < parts.size(); ++i)
        {
            const std::string& seg = parts[i];
            std::unique_ptr<AssetNode>& child = node->children[seg];

            if (!child)
            {
                std::unique_ptr<AssetNode> newNode = std::make_unique<AssetNode>();
                newNode->name = seg;
                if (node->path.empty())
                {
                    newNode->path = seg;
                }
                else
                {
                    newNode->path = node->path + "/" + seg;
                }
                child = std::move(newNode);
            }

            node = child.get();
        }

        // Create the asset item
        AssetItem item;
        item.uid = uid;
        item.name = GetFileNameFromPath(assetPath); // Display the actual filename
        item.type = getClassName(typeid(*res));
        item.res = res;

        node->items.push_back(std::move(item));
    }

    // Sort: folders by name, items by name
    std::function<void(AssetNode&)> sortRec;
    sortRec = [&](AssetNode& n)
    {
        for (auto& kv : n.children)
        {
            sortRec(*kv.second);
        }

        std::sort(n.items.begin(), n.items.end(),
            [](const AssetItem& a, const AssetItem& b)
            {
                return a.name < b.name;
            });
    };

    sortRec(*m_root);
}

AssetNode* AssetsBrowser::findNode(const std::string& path)
{
    if (!m_root)
    {
        return nullptr;
    }

    if (path.empty())
    {
        return m_root.get();
    }

    std::vector<std::string> parts = SplitPath(path);
    AssetNode* node = m_root.get();

    for (size_t i = 0; i < parts.size(); ++i)
    {
        const std::string& seg = parts[i];
        auto it = node->children.find(seg);

        if (it == node->children.end())
        {
            return m_root.get();
        }

        node = it->second.get();
    }

    return node;
}

void AssetsBrowser::SelectResource(const ResourcePtr& r)
{
    if (!r) return;
    Editor->SetSelectedSelectable(r);
    ResourceManager::GetInstance().SetSelectedResource(r);
}

void AssetsBrowser::drawLeftTree(AssetNode& node) {
    ImGuiTreeNodeFlags base =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth |
        ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
    
    ResourceManager& rm = ResourceManager::GetInstance();
    
    for (auto& [name, childPtr] : node.children) {
        auto& child = *childPtr;
        ImGuiTreeNodeFlags flags = base;
        if (child.children.empty() && child.items.empty())
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        bool open = ImGui::TreeNodeEx((child.path + "##tree").c_str(), flags, "%s", name.c_str());
        
        if (ImGui::IsItemClicked()) {
            m_currentFolder = child.path;
        }
        
        // Drop target - accept resources being moved here
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_RESOURCE"))
            {
                Resource* droppedResource = *static_cast<Resource**>(payload->Data);
                if (droppedResource != nullptr)
                {
                    ResourcePtr resourcePtr = rm.GetResourceByUniqueID(droppedResource->getUniqueID());
                    if (resourcePtr)
                    {
                        if (rm.MoveResource(resourcePtr, child.path))
                        {
                            m_needsRebuild = true;
                        }
                    }
                }
            }

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_GAMEOBJECT"))
            {
                GameObject* droppedGameObject = *static_cast<GameObject**>(payload->Data);
                if (droppedGameObject != nullptr)
                {
                    std::string prefabPath = child.path + "/" + droppedGameObject->name + ".prefab";

                    json prefabJson;
                    Prefab::recurseSerialize(droppedGameObject, prefabJson);
                    prefabJson["m_path"] = prefabPath;
                    prefabJson["type"] = "Prefab";
                    prefabJson["uniqueId"] = prefabPath;

                    rm.createResource("Prefab", prefabPath, prefabJson);
                    m_needsRebuild = true;
                }
            }

            ImGui::EndDragDropTarget();
        }
        
        if (open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
            drawLeftTree(child);
            ImGui::TreePop();
        }
        // context menu for folder
        if (ImGui::BeginPopupContextItem((child.path + "##folderctx").c_str())) {
            if (ImGui::MenuItem("Create Folder")) {
                // implement on-disk or virtual folder creation as needed
            }
            if (ImGui::MenuItem("Reveal in Explorer")) {
#ifdef _WIN32
                std::string abs = child.path;
                ShellExecuteA(nullptr, "open", abs.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#endif
            }
            ImGui::EndPopup();
        }
    }
}

void AssetsBrowser::drawRightPane(AssetNode& node)
{
    ResourceManager& resourceManager = ResourceManager::GetInstance();

    std::vector<std::string> parts = SplitPath(m_currentFolder);

    if (ImGui::SmallButton("Root"))
    {
        m_currentFolder = "";
    }

    if (!parts.empty())
    {
        ImGui::SameLine();
        ImGui::TextUnformatted(">");
    }

    std::string accumulatedPath;
    for (size_t i = 0; i < parts.size(); ++i)
    {
        ImGui::SameLine();
        accumulatedPath = accumulatedPath.empty() ? parts[i] : accumulatedPath + "/" + parts[i];
        if (ImGui::SmallButton(parts[i].c_str()))
        {
            m_currentFolder = accumulatedPath;
        }

        if (i + 1 < parts.size())
        {
            ImGui::SameLine();
            ImGui::TextUnformatted(">");
        }
    }

    ImGui::Separator();
    m_filter.Draw("Search", 200);
    ImGui::SameLine();
    ImGui::Checkbox("Grid", &m_showGrid);
    if (m_showGrid)
    {
        ImGui::SameLine();
        ImGui::SliderFloat("Size", &m_gridSize, 48.0f, 160.0f, "%.0f");
    }
    ImGui::Separator();

    ImGui::BeginChild("right_scroller");

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_GAMEOBJECT"))
        {
            GameObject* droppedGameObject = *static_cast<GameObject**>(payload->Data);
            if (droppedGameObject != nullptr)
            {
                std::string targetFolder = m_currentFolder;
                if (targetFolder.empty())
                {
                    targetFolder = "Assets";
                }

                std::string prefabFileName = droppedGameObject->name + ".prefab";
                std::string assetPath = JoinPath(targetFolder, prefabFileName);

                json prefabJson;
                Prefab::recurseSerialize(droppedGameObject, prefabJson);
                prefabJson["m_path"] = assetPath;
                prefabJson["type"] = "Prefab";
                prefabJson["uniqueId"] = assetPath;

                resourceManager.createResource("Prefab", assetPath, prefabJson);
                m_needsRebuild = true;
            }
        }

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_RESOURCE"))
        {
            Resource* droppedResource = *static_cast<Resource**>(payload->Data);
            if (droppedResource != nullptr && !m_currentFolder.empty())
            {
                ResourcePtr resourcePtr = resourceManager.GetResourceByUniqueID(droppedResource->getUniqueID());
                if (resourcePtr)
                {
                    if (resourceManager.MoveResource(resourcePtr, m_currentFolder))
                    {
                        m_needsRebuild = true;
                    }
                }
            }
        }

        ImGui::EndDragDropTarget();
    }

    for (auto& [folderName, childNodePtr] : node.children)
    {
        if (m_filter.IsActive() && !m_filter.PassFilter(folderName.c_str()))
        {
            continue;
        }

        if (ImGui::Selectable(("Folder " + folderName).c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
        {
            if (ImGui::IsMouseDoubleClicked(0))
            {
                m_currentFolder = childNodePtr->path;
            }
        }
    }

    if (m_showGrid)
    {
        float availableWidth = ImGui::GetContentRegionAvail().x;
        float cellSize = m_gridSize;
        float spacing = ImGui::GetStyle().ItemSpacing.x + 16.0f;
        int columns = max(1, static_cast<int>((availableWidth + spacing) / (cellSize + spacing)));
        int currentColumn = 0;

        auto advanceCell = [&]()
        {
            currentColumn += 1;
            if (currentColumn >= columns)
            {
                currentColumn = 0;
            }
            else
            {
                ImGui::SameLine();
                ImGui::Dummy(ImVec2(16.0f, 0.0f));
                ImGui::SameLine();
            }
        };

        for (auto& [folderName, childNodePtr] : node.children)
        {
            if (m_filter.IsActive() && !m_filter.PassFilter(folderName.c_str()))
            {
                continue;
            }

            AssetNode* childNode = childNodePtr.get();

            ImGui::BeginGroup();
            ImGui::Button(("Folder##" + childNode->path).c_str(), ImVec2(cellSize, cellSize));

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_RESOURCE"))
                {
                    Resource* droppedResource = *static_cast<Resource**>(payload->Data);
                    if (droppedResource != nullptr)
                    {
                        ResourcePtr resourcePtr = resourceManager.GetResourceByUniqueID(droppedResource->getUniqueID());
                        if (resourcePtr)
                        {
                            if (resourceManager.MoveResource(resourcePtr, childNode->path))
                            {
                                m_needsRebuild = true;
                            }
                        }
                    }
                }

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_GAMEOBJECT"))
                {
                    GameObject* droppedGameObject = *static_cast<GameObject**>(payload->Data);
                    if (droppedGameObject != nullptr)
                    {
                        std::string prefabFileName = droppedGameObject->name + ".prefab";
                        std::string assetPath = JoinPath(childNode->path, prefabFileName);

                        json prefabJson;
                        Prefab::recurseSerialize(droppedGameObject, prefabJson);
                        prefabJson["m_path"] = assetPath;
                        prefabJson["type"] = "Prefab";
                        prefabJson["uniqueId"] = assetPath;

                        resourceManager.createResource("Prefab", assetPath, prefabJson);
                        m_needsRebuild = true;
                    }
                }

                ImGui::EndDragDropTarget();
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                m_currentFolder = childNode->path;
            }

            ImGui::TextWrapped("%s", folderName.c_str());
            ImGui::EndGroup();

            advanceCell();
        }

        for (auto& item : node.items)
        {
            if (m_filter.IsActive())
            {
                bool passesFilter = false;
                if (m_filter.PassFilter(item.name.c_str()))
                {
                    passesFilter = true;
                }
                if (m_filter.PassFilter(item.type.c_str()))
                {
                    passesFilter = true;
                }
                if (m_filter.PassFilter(item.uid.c_str()))
                {
                    passesFilter = true;
                }

                if (!passesFilter)
                {
                    continue;
                }
            }

            ImGui::BeginGroup();
            ImGui::Button((item.type + "##btn" + item.uid).c_str(), ImVec2(cellSize, cellSize));

            if (ImGui::BeginDragDropSource())
            {
                Resource* rawResource = item.res.get();
                ImGui::SetDragDropPayload("DND_RESOURCE", &rawResource, sizeof(Resource*));
                ImGui::Text("%s", item.name.c_str());
                ImGui::EndDragDropSource();
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                SelectResource(item.res);
            }

            if (m_renameUID == item.uid)
            {
                ImGui::SetNextItemWidth(cellSize);
                ImGui::SetKeyboardFocusHere();
                if (ImGui::InputText(("##rename" + item.uid).c_str(), m_renameBuf, IM_ARRAYSIZE(m_renameBuf),
                    ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                {
                    if (m_renameBuf[0] != 0)
                    {
                        if (resourceManager.RenameResource(item.res, m_renameBuf))
                        {
                            m_needsRebuild = true;
                        }
                    }

                    m_renameUID.clear();
                }

                if (ImGui::IsKeyPressed(ImGuiKey_Escape) || (!ImGui::IsItemActive() && !ImGui::IsItemFocused()))
                {
                    m_renameUID.clear();
                }
            }
            else
            {
                ImGui::TextWrapped("%s", item.name.c_str());
            }

            ImGui::EndGroup();

            advanceCell();
        }
    }
    else
    {
        if (ImGui::BeginTable("assets_table", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort);
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Path");
            ImGui::TableHeadersRow();

            for (auto& item : node.items)
            {
                if (m_filter.IsActive() &&
                    !m_filter.PassFilter(item.name.c_str()) &&
                    !m_filter.PassFilter(item.type.c_str()) &&
                    !m_filter.PassFilter(item.uid.c_str()))
                {
                    continue;
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                bool isSelected = (item.res == resourceManager.GetSelectedResource());
                if (ImGui::Selectable((item.name + "##" + item.uid).c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
                {
                    SelectResource(item.res);
                }

                if (ImGui::BeginDragDropSource())
                {
                    Resource* rawResource = item.res.get();
                    ImGui::SetDragDropPayload("DND_RESOURCE", &rawResource, sizeof(Resource*));
                    ImGui::Text("%s", item.name.c_str());
                    ImGui::EndDragDropSource();
                }

                if (m_renameUID == item.uid)
                {
                    ImGui::SameLine();
                    ImGui::SetKeyboardFocusHere();
                    if (ImGui::InputText(("##rename" + item.uid).c_str(), m_renameBuf, IM_ARRAYSIZE(m_renameBuf),
                        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                    {
                        if (m_renameBuf[0] != 0)
                        {
                            if (resourceManager.RenameResource(item.res, m_renameBuf))
                            {
                                m_needsRebuild = true;
                            }
                        }

                        m_renameUID.clear();
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_Escape) || (!ImGui::IsItemActive() && !ImGui::IsItemFocused()))
                    {
                        m_renameUID.clear();
                    }
                }

                if (ImGui::BeginPopupContextItem((item.uid + "##ctx").c_str()))
                {
                    if (ImGui::MenuItem("Rename"))
                    {
                        m_renameUID = item.uid;
                        strncpy_s(m_renameBuf, item.name.c_str(), sizeof(m_renameBuf));
                        m_renameBuf[sizeof(m_renameBuf) - 1] = 0;
                    }

                    if (ImGui::MenuItem("Delete"))
                    {
                        std::string filePath = item.res->getPath();
                        if (std::filesystem::exists(filePath))
                        {
                            try
                            {
                                std::filesystem::remove(filePath);
                            }
                            catch (const std::exception& e)
                            {
                                Debug::LogError("Failed to delete file: " + std::string(e.what()), false);
                            }
                        }

                        resourceManager.DeleteResource(item.uid);
                        m_needsRebuild = true;
                    }

                    if (ImGui::MenuItem("Reveal in Explorer"))
                    {
#ifdef _WIN32
                        std::string absolutePath = item.res->getPath();
                        ShellExecuteA(nullptr, "open", absolutePath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#endif
                    }

                    ImGui::EndPopup();
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(item.type.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(item.uid.c_str());
            }

            ImGui::EndTable();
        }
    }

    ImGui::EndChild();
}

void AssetsBrowser::Draw() {
    if (!m_root) Rebuild();

    
    ResourceManager& rm = ResourceManager::GetInstance();

    uint64_t revision = rm.GetRevision();
    if (m_needsRebuild)
    {
        Rebuild();
        m_needsRebuild = false;
        m_lastRevision = revision;
    }
    else
    {
        if (revision != m_lastRevision)
        {
            Rebuild();
            m_lastRevision = revision;
        }
    }
    
    if (ImGui::Begin("Assets")) {
        // top bar create button
        if (ImGui::Button("Create")) ImGui::OpenPopup("CreateMenu");
        if (ImGui::BeginPopup("CreateMenu"))
        {
            for (auto& t : rm.GetCreatableResourceTypes())
            {
                if (ImGui::MenuItem(t.c_str()))
                {
                    selectedTypeName = t;
                    createResource = true;
                }
            }
            ImGui::EndPopup();
        }
        
        if (createResource) ImGui::OpenPopup("Enter Resource ID");
        if (ImGui::BeginPopupModal("Enter Resource ID", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            createResource = false;
            string label = selectedTypeName + " Filename";
            ImGui::InputText(label.c_str(), iDBuffer, sizeof(iDBuffer));
    
            if (ImGui::Button("Create"))
            {
                if (strlen(iDBuffer) > 0)
                {
                    const std::string fileName = std::string(iDBuffer);
                    const std::string assetPath = BuildResourceAssetPath(m_currentFolder, selectedTypeName, fileName);

                    json payload;
                    payload["m_path"] = assetPath;
                    
                    rm.createResource(selectedTypeName, fileName, payload);
                    m_needsRebuild = true;
                    
                    // Reset state
                    iDBuffer[0] = '\0';
                    selectedTypeName.clear();
    
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                selectedTypeName.clear();
                iDBuffer[0] = '\0';
            
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
        // panes with a splitter
        const float leftMin = 160.0f;
        static float leftWidth = 260.0f;
        float fullW = ImGui::GetContentRegionAvail().x;
        ImGui::BeginChild("left", ImVec2(leftWidth, 0), true);
        drawLeftTree(*m_root);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::InvisibleButton("splitter", ImVec2(4, ImGui::GetContentRegionAvail().y));
        if (ImGui::IsItemActive()) leftWidth = std::clamp(leftWidth + ImGui::GetIO().MouseDelta.x, leftMin, fullW - 200.0f);
        ImGui::SameLine();

        ImGui::BeginChild("right", ImVec2(0, 0), true);
        AssetNode* cur = findNode(m_currentFolder);
        drawRightPane(*cur);
        ImGui::EndChild();
    }
    ImGui::End();
}