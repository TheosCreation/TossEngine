#include "AssetBrowser.h"

#include <ObjectArray.h>

#include "ResourceManager.h"
#include "Resource.h"
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
        return "Assets";
    }
    return JoinPath("Assets", currentFolder);
}

static std::string BuildResourceAssetPath(const std::string& currentFolder, const std::string& typeName, const std::string& resourceId)
{
    const std::string folderPath = GetAssetsFolderPathFromCurrentFolder(currentFolder);
    const std::string extension = ResourceManager::GetExtensionForType(typeName);
    const std::string fileName = resourceId + extension;
    return JoinPath(folderPath, fileName);
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
    m_root->name = "Assets";
    m_root->path = "";

    const std::filesystem::path assetsRoot = std::filesystem::path("Assets");
    if (std::filesystem::exists(assetsRoot))
    {
        std::filesystem::recursive_directory_iterator endIt;
        std::filesystem::recursive_directory_iterator it(assetsRoot, std::filesystem::directory_options::skip_permission_denied);

        for (; it != endIt; ++it)
        {
            if (!it->is_directory())
            {
                continue;
            }

            std::filesystem::path dirPath = it->path();
            std::filesystem::path relPath = dirPath.lexically_relative(assetsRoot);
            std::string relFolder = relPath.generic_string(); // "" for root child? (first level folders become "Textures", etc.)

            EnsureFolderNode(*m_root, relFolder);
        }
    }

    // Now layer imported/known resources into the folder nodes
    ResourceManager& rm = ResourceManager::GetInstance();
    std::map<std::string, ResourcePtr>& map = rm.GetAllResources();

    for (auto& kv : map)
    {
        const std::string& uid = kv.first;
        const ResourcePtr& res = kv.second;

        if (uid.empty())
        {
            continue;
        }

        std::vector<std::string> parts = SplitPath(uid);
        AssetNode* node = m_root.get();

        // walk/create folders for all but last token
        for (size_t i = 0; i + 1 < parts.size(); ++i)
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

        AssetItem item;
        item.uid = uid;
        if (parts.empty())
        {
            item.name = uid;
        }
        else
        {
            item.name = parts.back();
        }
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
    for (auto& [name, childPtr] : node.children) {
        auto& child = *childPtr;
        ImGuiTreeNodeFlags flags = base;
        if (child.children.empty() && child.items.empty())
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        bool open = ImGui::TreeNodeEx((child.path + "##tree").c_str(), flags, "%s", name.c_str());
        if (ImGui::IsItemClicked()) {
            m_currentFolder = child.path;
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
                std::string abs = std::string("Assets/") + child.path;
                ShellExecuteA(nullptr, "open", abs.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#endif
            }
            ImGui::EndPopup();
        }
    }
}

void AssetsBrowser::drawRightPane(AssetNode& node) {
    ResourceManager& rm = ResourceManager::GetInstance();
    // Breadcrumb
    ImGui::TextUnformatted("Assets"); ImGui::SameLine();
    std::string accum;
    auto parts = SplitPath(m_currentFolder);
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i) ImGui::SameLine();
        accum = accum.empty() ? parts[i] : accum + "/" + parts[i];
        if (ImGui::SmallButton(parts[i].c_str())) m_currentFolder = accum;
        if (i + 1 < parts.size()) { ImGui::SameLine(); ImGui::TextUnformatted(">"); }
    }

    // Toolbar
    ImGui::Separator();
    m_filter.Draw("Search", 200);
    ImGui::SameLine();
    ImGui::Checkbox("Grid", &m_showGrid);
    if (m_showGrid) { ImGui::SameLine(); ImGui::SliderFloat("Size", &m_gridSize, 48, 160, "%.0f"); }
    ImGui::Separator();

    // Child region for scrolling
    ImGui::BeginChild("right_scroller");

    // Show subfolders first
    for (auto& [fname, ch] : node.children) {
        if (m_filter.IsActive() && !m_filter.PassFilter(fname.c_str())) continue;
        if (ImGui::Selectable(("Folder " + fname).c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
            if (ImGui::IsMouseDoubleClicked(0)) m_currentFolder = ch->path;
        }
    }

    // Files
    if (m_showGrid)
    {
        float avail = ImGui::GetContentRegionAvail().x;
        float cell = m_gridSize;
        float spacing = ImGui::GetStyle().ItemSpacing.x + 16.0f;
        int cols = max(1, (int)((avail + spacing) / (cell + spacing)));
        int col = 0;
    
        auto AdvanceCell = [&]()
        {
            col += 1;
            if (col >= cols)
            {
                col = 0;
            }
            else
            {
                ImGui::SameLine();
                ImGui::Dummy(ImVec2(16.0f, 0.0f));
                ImGui::SameLine();
            }
        };
    
        // Folders
        for (auto& kv : node.children)
        {
            const std::string& fname = kv.first;
            AssetNode* childNode = kv.second.get();
    
            if (m_filter.IsActive())
            {
                if (!m_filter.PassFilter(fname.c_str()))
                {
                    continue;
                }
            }
    
            ImGui::BeginGroup();
            ImGui::Button(("Folder##" + childNode->path).c_str(), ImVec2(cell, cell));
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                m_currentFolder = childNode->path;
            }
            ImGui::TextWrapped("%s", fname.c_str());
            ImGui::EndGroup();
    
            AdvanceCell();
        }
    
        // Assets
        for (auto& it : node.items)
        {
            if (m_filter.IsActive())
            {
                bool pass = false;
                if (m_filter.PassFilter(it.name.c_str()))
                {
                    pass = true;
                }
                if (m_filter.PassFilter(it.type.c_str()))
                {
                    pass = true;
                }
                if (!pass)
                {
                    continue;
                }
            }
    
            ImGui::BeginGroup();
            ImGui::Button((it.type + "##btn" + it.uid).c_str(), ImVec2(cell, cell));
    
            if (ImGui::BeginDragDropSource())
            {
                Resource* raw = it.res.get();
                ImGui::SetDragDropPayload("DND_RESOURCE", &raw, sizeof(raw));
                ImGui::Text("%s", it.uid.c_str());
                ImGui::EndDragDropSource();
            }
    
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                SelectResource(it.res);
            }
    
            if (m_renameUID == it.uid)
            {
                ImGui::SetNextItemWidth(cell);
                if (ImGui::InputText(("##rename" + it.uid).c_str(), m_renameBuf, IM_ARRAYSIZE(m_renameBuf),
                    ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (m_renameBuf[0] != 0)
                    {
                        rm.RenameResource(it.res, std::string(m_renameBuf));
                    }
                    m_renameUID.clear();
                }
            }
            else
            {
                ImGui::TextWrapped("%s", it.name.c_str());
            }
    
            ImGui::EndGroup();
    
            AdvanceCell();
        }
    }
    else {
        // list view with table
        if (ImGui::BeginTable("assets_table", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort);
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("UID");
            ImGui::TableHeadersRow();

            // sortable: for brevity we reuse existing sorted order

            for (auto& it : node.items) {
                if (m_filter.IsActive() && !m_filter.PassFilter(it.name.c_str()) && !m_filter.PassFilter(it.type.c_str()) && !m_filter.PassFilter(it.uid.c_str()))
                    continue;

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                bool selected = (it.res == rm.GetSelectedResource());
                if (ImGui::Selectable((it.type + it.name + "##" + it.uid).c_str(), selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    SelectResource(it.res);
                }
                if (ImGui::BeginDragDropSource()) {
                    Resource* raw = it.res.get();
                    ImGui::SetDragDropPayload("DND_RESOURCE", &raw, sizeof(raw));
                    ImGui::Text("%s", it.uid.c_str());
                    ImGui::EndDragDropSource();
                }

                // inline rename
                if (m_renameUID == it.uid) {
                    ImGui::SameLine();
                    if (ImGui::InputText(("##rename" + it.uid).c_str(), m_renameBuf, IM_ARRAYSIZE(m_renameBuf),
                        ImGuiInputTextFlags_EnterReturnsTrue)) {
                        if (m_renameBuf[0]) rm.RenameResource(it.res, std::string(m_renameBuf));
                        m_renameUID.clear();
                    }
                }

                // context
                if (ImGui::BeginPopupContextItem((it.uid + "##ctx").c_str())) {
                    if (ImGui::MenuItem("Rename")) {
                        m_renameUID = it.uid;
                        strncpy_s(m_renameBuf, it.name.c_str(), sizeof(m_renameBuf));
                        m_renameBuf[sizeof(m_renameBuf) - 1] = 0;
                    }
                    if (ImGui::MenuItem("Delete")) rm.DeleteResource(it.uid);
                    if (ImGui::MenuItem("Reveal in Explorer")) {
#ifdef _WIN32
                        std::string abs = std::string("Assets/") + it.uid;
                        ShellExecuteA(nullptr, "open", abs.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#endif
                    }
                    ImGui::EndPopup();
                }

                ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(it.type.c_str());
                ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(it.uid.c_str());
            }
            ImGui::EndTable();
        }
    }

    ImGui::EndChild();
}

void AssetsBrowser::Draw() {
    if (!m_root) Rebuild();

    
    ResourceManager& rm = ResourceManager::GetInstance();

    
    
    if (ImGui::Begin("Assets")) {
        // top bar create button
        if (ImGui::Button("Create")) ImGui::OpenPopup("CreateMenu");
        if (ImGui::BeginPopup("CreateMenu"))
        {
            for (auto& t : rm.GetCreatableResourceTypes())
            {
                if (ImGui::MenuItem(t.c_str()))
                {
                    // open your existing "Enter Resource ID" modal
                    selectedTypeName = t;
                    createResource = true;
                    //ImGui::OpenPopup("Enter Resource ID");
                }
            }
            ImGui::EndPopup();
        }
        
        if (createResource) ImGui::OpenPopup("Enter Resource ID");
        if (ImGui::BeginPopupModal("Enter Resource ID", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            createResource = false;
            string label = selectedTypeName + " ID";
            ImGui::InputText(label.c_str(), iDBuffer, sizeof(iDBuffer));
    
            if (ImGui::Button("Create"))
            {
                if (strlen(iDBuffer) > 0)
                {
                    const std::string resourceId = std::string(iDBuffer);
                    const std::string assetPath = BuildResourceAssetPath(m_currentFolder, selectedTypeName, resourceId);

                    json payload;
                    payload["m_path"] = assetPath;
                    rm.createResource(selectedTypeName, iDBuffer, payload);
    
                    // Reset state
                    iDBuffer[0] = '\0';
                    selectedTypeName.clear();
    
                    ImGui::CloseCurrentPopup();
                }
            }

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

    // process deletions and keep tree in sync
    rm.onUpdateInternal();
    // If resources changed externally, call Rebuild(rm)
}
