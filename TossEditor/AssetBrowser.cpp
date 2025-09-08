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

AssetsBrowser::AssetsBrowser(TossEditor* editor)
{
    Editor = editor;
}

AssetsBrowser::~AssetsBrowser()
{
}

void AssetsBrowser::Rebuild() {
    m_root = std::make_unique<AssetNode>();
    m_root->name = "Assets";
    m_root->path = "";

    ResourceManager& rm = ResourceManager::GetInstance();
    auto& map = rm.GetAllResources();
    for (auto& [uid, res] : map) {
        if (uid.empty()) continue;
        auto parts = SplitPath(uid);                   // ["Textures","ui","button.png"]
        AssetNode* node = m_root.get();

        // walk/create folders for all but last token
        for (size_t i = 0; i + 1 < parts.size(); ++i) {
            auto& seg = parts[i];
            auto& child = node->children[seg];
            if (!child) {
                child = std::make_unique<AssetNode>();
                child->name = seg;
                child->path = node->path.empty() ? seg : (node->path + "/" + seg);
            }
            node = child.get();
        }
        // file item
        AssetItem item;
        item.uid = uid;
        item.name = parts.empty() ? uid : parts.back();
        item.type = getClassName(typeid(*res));
        item.res = res;
        node->items.push_back(std::move(item));
    }

    // optional: sort folders and files
    std::function<void(AssetNode&)> sortRec = [&](AssetNode& n) {
        for (auto& [k, ch] : n.children) sortRec(*ch);
        std::sort(n.items.begin(), n.items.end(), [](auto& a, auto& b) { return a.name < b.name; });
        };
    sortRec(*m_root);
}

AssetNode* AssetsBrowser::findNode(const std::string& path) {
    if (!m_root) return nullptr;
    if (path.empty()) return m_root.get();
    auto parts = SplitPath(path);
    AssetNode* node = m_root.get();
    for (auto& seg : parts) {
        auto it = node->children.find(seg);
        if (it == node->children.end()) return node; // fallback
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
    if (m_showGrid) {
        // simple flow layout
        float avail = ImGui::GetContentRegionAvail().x;
        float cell = m_gridSize;
        float spacing = ImGui::GetStyle().ItemSpacing.x + 16.0f;
        int cols = max(1, (int)((avail + spacing) / (cell + spacing)));
        int col = 0;

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        for (auto& it : node.items) {
            if (m_filter.IsActive() && !m_filter.PassFilter(it.name.c_str()) && !m_filter.PassFilter(it.type.c_str()))
                continue;

            ImGui::BeginGroup();
            ImGui::Button((it.type + "##btn" + it.uid).c_str(), ImVec2(cell, cell)); // replace with thumbnail if available
            if (ImGui::BeginDragDropSource()) {
                Resource* raw = it.res.get();
                ImGui::SetDragDropPayload("DND_RESOURCE", &raw, sizeof(raw));
                ImGui::Text("%s", it.uid.c_str());
                ImGui::EndDragDropSource();
            }

            // selection + open
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                SelectResource(it.res);
            }

            // label or inline-rename
            if (m_renameUID == it.uid) {
                ImGui::SetNextItemWidth(cell);
                if (ImGui::InputText(("##rename" + it.uid).c_str(), m_renameBuf, IM_ARRAYSIZE(m_renameBuf),
                    ImGuiInputTextFlags_EnterReturnsTrue)) {
                    if (m_renameBuf[0]) {
                        rm.RenameResource(it.res, std::string(m_renameBuf));
                    }
                    m_renameUID.clear();
                }
            }
            else {
                ImGui::TextWrapped("%s", it.name.c_str());
            }

            // context
            if (ImGui::BeginPopupContextItem((it.uid + "##itemctx").c_str())) {
                if (ImGui::MenuItem("Rename")) {
                    m_renameUID = it.uid;
                    strncpy_s(m_renameBuf, it.name.c_str(), sizeof(m_renameBuf));
                    m_renameBuf[sizeof(m_renameBuf) - 1] = 0;
                }
                if (ImGui::MenuItem("Delete")) {
                    rm.DeleteResource(it.uid);
                }
                if (ImGui::MenuItem("Select")) {
                    SelectResource(it.res);
                }
                ImGui::EndPopup();
            }
            ImGui::EndGroup();

            if (++col >= cols) { col = 0; }
            else { ImGui::SameLine(); ImGui::Dummy(ImVec2(16, 0)); ImGui::SameLine(); }
        }
        ImGui::PopStyleVar();
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
        if (ImGui::BeginPopup("CreateMenu")) {
            for (auto& t : rm.GetCreatableResourceTypes()) {
                if (ImGui::MenuItem(t.c_str())) {
                    // open your existing "Enter Resource ID" modal
                    // set selectedTypeName = t, etc.
                }
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
