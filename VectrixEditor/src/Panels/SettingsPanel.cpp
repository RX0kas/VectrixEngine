#include "SettingsPanel.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"

#include "Vectrix/Application.h"
#include "Vectrix/Settings/Outline.h"
#include "Vectrix/Settings/SettingsManager.h"

namespace Vectrix {
    namespace {
        // Sidebar leaves grouped into collapsible sections. `id` is the stable index used
        // by render()'s switch and by the persisted "settingsCategory" — do not renumber.
        struct Leaf { int id; const char* label; };
        constexpr Leaf kEditorLeaves[] = {
            {0, "Outline"}, {1, "Camera"}, {2, "Gizmo"}, {3, "Content Browser"}, {4, "Editor UI"}
        };
        constexpr Leaf kEngineLeaves[] = {
            {5, "Rendering"}, {6, "Window"}, {7, "Logging"}
        };
        constexpr Leaf kVulkanLeaves[] = {
            {8, "Swapchain"}, {9, "Device"}, {10, "Textures"}, {11, "Shaders"}, {12, "Pipeline"}
        };
        constexpr int kCategoryCount =
            IM_ARRAYSIZE(kEditorLeaves) + IM_ARRAYSIZE(kEngineLeaves) + IM_ARRAYSIZE(kVulkanLeaves);

        // Option lists and defaults mirror the engine defaults in
        // Vectrix/src/GraphicAPI/Vulkan/VulkanSettings.h and the hardcoded values they
        // replaced elsewhere — keep them in sync.
        constexpr const char* kPresentModes[] = { "fifo", "mailbox", "immediate", "fifoRelaxed" };
        constexpr int         kPresentModeDefault = 1; // mailbox
        constexpr const char* kSeverities[]   = { "verbose", "info", "warning", "error" };
        constexpr int         kSeverityDefault = 2; // warning
        constexpr const char* kFilters[]      = { "linear", "nearest" };
        constexpr const char* kWrapModes[]    = { "repeat", "mirroredRepeat", "clampEdge", "clampBorder" };
        constexpr const char* kThemes[]       = { "dark", "light" };
        constexpr const char* kLogLevels[]    = { "trace", "debug", "info", "warning", "error", "critical", "off" };

        std::string toLower(std::string value) {
            std::transform(value.begin(), value.end(), value.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return value;
        }

        std::vector<const char*> keyPath(std::vector<const char*> base, const char* key) {
            base.push_back(key);
            return base;
        }

        // Read from the merged view: any missing link yields a null value, never inserts.
        const JsonValue& readAt(const JsonObject& root, const std::vector<const char*>& path) {
            static const JsonValue s_null;
            if (path.empty()) return s_null;
            const auto it = root.find(path.front());
            if (it == root.end()) return s_null;
            const JsonValue* current = &it->second;
            for (std::size_t i = 1; i < path.size(); ++i)
                current = &(*current)[path[i]];
            return *current;
        }

        // Write into one tier, creating every object along the way. Only call on a real change.
        JsonValue& writeAt(JsonObject& root, const std::vector<const char*>& path) {
            JsonValue* current = &root[path.front()];
            for (std::size_t i = 1; i < path.size(); ++i)
                current = &(*current)[path[i]];
            return *current;
        }

        // Each control shows the effective (merged) value and writes edits into the target tier.
        bool comboEnum(const JsonObject& read, JsonObject& write, const std::vector<const char*>& path,
                       const char* label, const char* const* options, int count, int defaultIdx) {
            int index = defaultIdx;
            if (const auto raw = readAt(read, path).getAs<std::string>()) {
                for (int i = 0; i < count; ++i) {
                    if (*raw == options[i]) { index = i; break; }
                }
            }
            if (ImGui::Combo(label, &index, options, count)) {
                writeAt(write, path) = options[index];
                return true;
            }
            return false;
        }

        bool checkbox(const JsonObject& read, JsonObject& write, const std::vector<const char*>& path,
                      const char* label, bool def) {
            bool value = readAt(read, path).getAs<bool>().value_or(def);
            if (ImGui::Checkbox(label, &value)) {
                writeAt(write, path) = value;
                return true;
            }
            return false;
        }

        bool dragInt(const JsonObject& read, JsonObject& write, const std::vector<const char*>& path,
                     const char* label, int def, int vmin, int vmax) {
            int value = readAt(read, path).getAs<int>().value_or(def);
            ImGui::DragInt(label, &value, 1.0f, vmin, vmax);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                writeAt(write, path) = static_cast<double>(std::clamp(value, vmin, vmax));
                return true;
            }
            return false;
        }

        bool dragFloat(const JsonObject& read, JsonObject& write, const std::vector<const char*>& path,
                       const char* label, double def, float speed, float vmin, float vmax) {
            auto value = static_cast<float>(readAt(read, path).getAs<double>().value_or(def));
            ImGui::DragFloat(label, &value, speed, vmin, vmax);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                writeAt(write, path) = static_cast<double>(value);
                return true;
            }
            return false;
        }

        bool sliderFloat(const JsonObject& read, JsonObject& write, const std::vector<const char*>& path,
                         const char* label, float def, float vmin, float vmax) {
            auto value = static_cast<float>(readAt(read, path).getAs<double>().value_or(def));
            ImGui::SliderFloat(label, &value, vmin, vmax, "%.0f");
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                writeAt(write, path) = static_cast<double>(value);
                return true;
            }
            return false;
        }

        bool inputText(const JsonObject& read, JsonObject& write, const std::vector<const char*>& path,
                       const char* label, const char* def) {
            char buffer[256];
            std::snprintf(buffer, sizeof(buffer), "%s", readAt(read, path).getAs<std::string>().value_or(def).c_str());
            ImGui::InputText(label, buffer, sizeof(buffer));
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                writeAt(write, path) = buffer;
                return true;
            }
            return false;
        }

        bool colorEdit4(const JsonObject& read, JsonObject& write, const std::vector<const char*>& path,
                        const char* label, const float def[4]) {
            const JsonValue& node = readAt(read, path);
            float rgba[4] = { def[0], def[1], def[2], def[3] };
            for (int i = 0; i < 4 && i < static_cast<int>(node.size()); ++i)
                rgba[i] = static_cast<float>(node[static_cast<std::size_t>(i)].getAs<double>().value_or(rgba[i]));
            if (ImGui::ColorEdit4(label, rgba)) {
                JsonArray array;
                for (const float v : rgba)
                    array.emplace_back(static_cast<double>(v));
                writeAt(write, path) = array;
                return true;
            }
            return false;
        }

        void restartNote() {
            ImGui::Spacing();
            ImGui::TextDisabled("Changes apply on the next launch.");
        }

        // ---- sections -------------------------------------------------------------------

        bool drawOutline(JsonObject& write) {
            ImGui::SeparatorText("Outline");

            // The renderer samples outlineSettings every frame, so editing it here is live;
            // the settings file is only rewritten once an edit ends.
            bool changed = false;

            ImGui::DragFloat("Thickness", &outlineSettings.thickness, 0.1f, 0.0f, 64.0f);
            changed |= ImGui::IsItemDeactivatedAfterEdit();

            ImGui::DragFloat2("Texel Size", glm::value_ptr(outlineSettings.texelSize), 0.1f, 0.0f, 64.0f);
            changed |= ImGui::IsItemDeactivatedAfterEdit();

            ImGui::ColorEdit4("Color", glm::value_ptr(outlineSettings.color));
            changed |= ImGui::IsItemDeactivatedAfterEdit();

            ImGui::Spacing();
            if (ImGui::SmallButton("Reset to defaults##outline")) {
                outlineSettings = OutlineSettings{};
                changed = true;
            }

            if (changed)
                writeOutlineSettings(write["editor"]["outline"]);

            return changed;
        }

        bool drawCamera(const JsonObject& r, JsonObject& w) {
            ImGui::SeparatorText("Editor camera");
            const std::vector<const char*> base = {"editor", "camera"};

            bool changed = false;
            changed |= dragFloat(r, w, keyPath(base, "moveSpeed"), "Move speed", 1.5, 0.05f, 0.0f, 100.0f);
            changed |= dragFloat(r, w, keyPath(base, "rotationSpeed"), "Rotation speed", 50.0, 0.5f, 0.0f, 720.0f);

            ImGui::Spacing();
            changed |= dragFloat(r, w, keyPath(base, "fov"), "Field of view", 50.0, 0.5f, 1.0f, 179.0f);
            changed |= dragFloat(r, w, keyPath(base, "near"), "Near plane", 0.1, 0.01f, 0.001f, 100.0f);
            changed |= dragFloat(r, w, keyPath(base, "far"), "Far plane", 1000.0, 5.0f, 1.0f, 100000.0f);
            ImGui::Spacing();
            ImGui::TextDisabled("Field of view and clip planes apply on the next launch.");

            ImGui::Spacing();
            if (ImGui::SmallButton("Reset to defaults##camera")) {
                writeAt(w, keyPath(base, "moveSpeed")) = 1.5;
                writeAt(w, keyPath(base, "rotationSpeed")) = 50.0;
                writeAt(w, keyPath(base, "fov")) = 50.0;
                writeAt(w, keyPath(base, "near")) = 0.1;
                writeAt(w, keyPath(base, "far")) = 1000.0;
                changed = true;
            }
            return changed;
        }

        bool drawGizmo(const JsonObject& r, JsonObject& w) {
            ImGui::SeparatorText("Gizmo");
            const std::vector<const char*> base = {"editor", "gizmo"};

            bool changed = false;
            changed |= dragFloat(r, w, keyPath(base, "translationSnap"), "Translation / scale snap", 0.5, 0.05f, 0.0f, 16.0f);
            changed |= dragFloat(r, w, keyPath(base, "rotationSnap"), "Rotation snap (deg)", 45.0, 1.0f, 0.0f, 180.0f);

            ImGui::Spacing();
            if (ImGui::SmallButton("Reset to defaults##gizmo")) {
                writeAt(w, keyPath(base, "translationSnap")) = 0.5;
                writeAt(w, keyPath(base, "rotationSnap")) = 45.0;
                changed = true;
            }
            return changed;
        }

        bool drawContentBrowser(const JsonObject& r, JsonObject& w) {
            ImGui::SeparatorText("Content browser");
            const std::vector<const char*> base = {"editor", "contentBrowser"};

            bool changed = false;
            changed |= sliderFloat(r, w, keyPath(base, "thumbnailSize"), "Thumbnail size", 84.0f, 48.0f, 160.0f);
            changed |= dragFloat(r, w, keyPath(base, "padding"), "Padding", 12.0, 0.5f, 0.0f, 64.0f);
            restartNote();

            ImGui::Spacing();
            if (ImGui::SmallButton("Reset to defaults##contentbrowser")) {
                writeAt(w, keyPath(base, "thumbnailSize")) = 84.0;
                writeAt(w, keyPath(base, "padding")) = 12.0;
                changed = true;
            }
            return changed;
        }

        bool drawEditorUi(const JsonObject& r, JsonObject& w) {
            ImGui::SeparatorText("Editor UI");
            const std::vector<const char*> base = {"editor", "ui"};

            bool changed = false;
            changed |= comboEnum(r, w, keyPath(base, "theme"), "Theme", kThemes, IM_ARRAYSIZE(kThemes), 0);

            ImGui::Spacing();
            changed |= checkbox(r, w, keyPath(base, "docking"), "Docking", true);
            changed |= checkbox(r, w, keyPath(base, "viewports"), "Multi-viewports", true);
            changed |= checkbox(r, w, keyPath(base, "dpiScaling"), "DPI scaling", true);
            ImGui::Spacing();
            ImGui::TextDisabled("Docking, viewports and DPI scaling apply on the next launch.");

            ImGui::Spacing();
            if (ImGui::SmallButton("Reset to defaults##editorui")) {
                writeAt(w, keyPath(base, "theme")) = kThemes[0];
                writeAt(w, keyPath(base, "docking")) = true;
                writeAt(w, keyPath(base, "viewports")) = true;
                writeAt(w, keyPath(base, "dpiScaling")) = true;
                changed = true;
            }
            return changed;
        }

        bool drawRendering(const JsonObject& r, JsonObject& w) {
            ImGui::SeparatorText("Rendering");
            const std::vector<const char*> base = {"engine", "rendering"};
            static constexpr float kClearDefault[4] = {0.0f, 0.0f, 0.0f, 1.0f};

            bool changed = colorEdit4(r, w, keyPath(base, "clearColor"), "Clear color", kClearDefault);

            ImGui::Spacing();
            if (ImGui::SmallButton("Reset to defaults##rendering")) {
                JsonArray black;
                for (const float v : kClearDefault)
                    black.emplace_back(static_cast<double>(v));
                writeAt(w, keyPath(base, "clearColor")) = black;
                changed = true;
            }
            return changed;
        }

        bool drawWindow(const JsonObject& r, JsonObject& w) {
            ImGui::SeparatorText("Window");
            const std::vector<const char*> base = {"window"};

            bool changed = false;
            changed |= checkbox(r, w, keyPath(base, "resizable"), "Resizable", false);
            changed |= dragInt(r, w, keyPath(base, "width"), "Width", 1280, 320, 7680);
            changed |= dragInt(r, w, keyPath(base, "height"), "Height", 720, 240, 4320);
            changed |= inputText(r, w, keyPath(base, "title"), "Title", "VectrixEditor");
            restartNote();

            ImGui::Spacing();
            if (ImGui::SmallButton("Reset to defaults##window")) {
                writeAt(w, keyPath(base, "resizable")) = false;
                writeAt(w, keyPath(base, "width")) = 1280.0;
                writeAt(w, keyPath(base, "height")) = 720.0;
                writeAt(w, keyPath(base, "title")) = "VectrixEditor";
                changed = true;
            }
            return changed;
        }

        bool drawLogging(const JsonObject& r, JsonObject& w) {
            ImGui::SeparatorText("Logging");
            const std::vector<const char*> base = {"engine", "logging"};

            bool changed = comboEnum(r, w, keyPath(base, "level"), "Level",
                                     kLogLevels, IM_ARRAYSIZE(kLogLevels), 0);
            restartNote();

            ImGui::Spacing();
            if (ImGui::SmallButton("Reset to defaults##logging")) {
                writeAt(w, keyPath(base, "level")) = kLogLevels[0];
                changed = true;
            }
            return changed;
        }

        bool drawSwapchain(const JsonObject& r, JsonObject& w) {
            ImGui::SeparatorText("Swapchain");
            const std::vector<const char*> base = {"engine", "graphicAPI", "vulkan", "swapchain"};

            bool changed = false;
            changed |= comboEnum(r, w, keyPath(base, "presentMode"), "Present mode",
                                 kPresentModes, IM_ARRAYSIZE(kPresentModes), kPresentModeDefault);
            changed |= dragInt(r, w, keyPath(base, "imageCount"), "Image count (0 = auto)", 0, 0, 8);
            restartNote();

            ImGui::Spacing();
            if (ImGui::SmallButton("Reset to defaults##swapchain")) {
                writeAt(w, keyPath(base, "presentMode")) = kPresentModes[kPresentModeDefault];
                writeAt(w, keyPath(base, "imageCount")) = 0.0;
                changed = true;
            }
            return changed;
        }

        bool drawDevice(const JsonObject& r, JsonObject& w) {
            ImGui::SeparatorText("Device");
            const std::vector<const char*> base = {"engine", "graphicAPI", "vulkan", "device"};

            bool changed = false;
            changed |= checkbox(r, w, keyPath(base, "preferDiscreteGpu"), "Prefer discrete GPU", true);
            changed |= inputText(r, w, keyPath(base, "preferredGpuName"), "Preferred GPU name", "");
            changed |= checkbox(r, w, keyPath(base, "validationLayers"), "Validation layers", true);
            changed |= comboEnum(r, w, keyPath(base, "validationLayerSeverity"), "Validation severity",
                                 kSeverities, IM_ARRAYSIZE(kSeverities), kSeverityDefault);
            restartNote();

            ImGui::Spacing();
            if (ImGui::SmallButton("Reset to defaults##device")) {
                writeAt(w, keyPath(base, "preferDiscreteGpu")) = true;
                writeAt(w, keyPath(base, "preferredGpuName")) = "";
                writeAt(w, keyPath(base, "validationLayers")) = true;
                writeAt(w, keyPath(base, "validationLayerSeverity")) = kSeverities[kSeverityDefault];
                changed = true;
            }
            return changed;
        }

        bool drawTextures(const JsonObject& r, JsonObject& w) {
            ImGui::SeparatorText("Textures");
            const std::vector<const char*> base = {"engine", "graphicAPI", "vulkan", "textures"};

            bool changed = false;
            changed |= comboEnum(r, w, keyPath(base, "filter"), "Filter",
                                 kFilters, IM_ARRAYSIZE(kFilters), 0);
            changed |= comboEnum(r, w, keyPath(base, "wrapMode"), "Wrap mode",
                                 kWrapModes, IM_ARRAYSIZE(kWrapModes), 0);
            changed |= sliderFloat(r, w, keyPath(base, "anisotropy"), "Anisotropy (0 = off)", 16.0f, 0.0f, 16.0f);
            restartNote();

            ImGui::Spacing();
            if (ImGui::SmallButton("Reset to defaults##textures")) {
                writeAt(w, keyPath(base, "filter")) = kFilters[0];
                writeAt(w, keyPath(base, "wrapMode")) = kWrapModes[0];
                writeAt(w, keyPath(base, "anisotropy")) = 16.0;
                changed = true;
            }
            return changed;
        }

        bool drawShaders(const JsonObject& r, JsonObject& w) {
            ImGui::SeparatorText("Shaders");
            const std::vector<const char*> base = {"engine", "graphicAPI", "vulkan", "shaders"};

            bool changed = checkbox(r, w, keyPath(base, "optimize"), "Optimize (shaderc)", true);
            restartNote();

            ImGui::Spacing();
            if (ImGui::SmallButton("Reset to defaults##shaders")) {
                writeAt(w, keyPath(base, "optimize")) = true;
                changed = true;
            }
            return changed;
        }

        bool drawPipeline(const JsonObject& r, JsonObject& w) {
            ImGui::SeparatorText("Pipeline");
            const std::vector<const char*> base = {"engine", "graphicAPI", "vulkan", "rendering"};

            bool changed = false;
            changed |= checkbox(r, w, keyPath(base, "backfaceCulling"), "Backface culling", true);
            changed |= checkbox(r, w, keyPath(base, "wireframe"), "Wireframe", false);
            restartNote();

            ImGui::Spacing();
            if (ImGui::SmallButton("Reset to defaults##pipeline")) {
                writeAt(w, keyPath(base, "backfaceCulling")) = true;
                writeAt(w, keyPath(base, "wireframe")) = false;
                changed = true;
            }
            return changed;
        }
    }

    SettingsPanel::SettingsPanel() : ImGuiWidget("SettingsPanel") {}

    void SettingsPanel::render() {
        if (!ImGui::Begin(getName().c_str(), &getEnable())) {
            ImGui::End();
            return;
        }

        const JsonObject& effective = SettingsManager::getSettings();
        const std::vector<const char*> categoryKey = {"editor", "ui", "settingsCategory"};

        if (m_selectedCategory < 0) {
            const int stored = readAt(effective, categoryKey).getAs<int>().value_or(0);
            m_selectedCategory = std::clamp(stored, 0, kCategoryCount - 1);
        }

        // Which tier edits land in.
        const bool hasProject = SettingsManager::hasProject();
        if (!hasProject)
            m_saveScope = 1; // no project -> Global only
        static const char* const kScopeItems[] = { "Project", "Global" };
        ImGui::SetNextItemWidth(160.0f);
        ImGui::BeginDisabled(!hasProject);
        ImGui::Combo("Edit tier", &m_saveScope, kScopeItems, IM_ARRAYSIZE(kScopeItems));
        ImGui::EndDisabled();
        ImGui::TextDisabled("%s", hasProject
            ? "Fields left unset here fall back to Global, then the built-in default."
            : "No project open - editing the Global settings.");

        const SettingsManager::Scope scope =
            m_saveScope == 0 ? SettingsManager::Scope::Project : SettingsManager::Scope::Global;
        JsonObject& target = SettingsManager::tier(scope);

        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputTextWithHint("##settingsSearch", "Search settings...", m_search, sizeof(m_search));
        const std::string query = toLower(m_search);

        const auto matches = [&](const char* label) {
            return query.empty() || toLower(label).find(query) != std::string::npos;
        };
        const auto groupMatches = [&](const Leaf* leaves, int count) {
            for (int i = 0; i < count; ++i)
                if (matches(leaves[i].label)) return true;
            return false;
        };
        const auto drawLeaves = [&](const Leaf* leaves, int count) {
            for (int i = 0; i < count; ++i) {
                if (!matches(leaves[i].label)) continue;
                if (ImGui::Selectable(leaves[i].label, m_selectedCategory == leaves[i].id))
                    m_selectedCategory = leaves[i].id;
            }
        };

        ImGui::BeginChild("##settingsCategories", ImVec2(190.0f, 0.0f), true);

        if (groupMatches(kEditorLeaves, IM_ARRAYSIZE(kEditorLeaves)) &&
            ImGui::CollapsingHeader("Editor", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent();
            drawLeaves(kEditorLeaves, IM_ARRAYSIZE(kEditorLeaves));
            ImGui::Unindent();
        }

        const bool engineMatch = groupMatches(kEngineLeaves, IM_ARRAYSIZE(kEngineLeaves));
        const bool vulkanMatch = groupMatches(kVulkanLeaves, IM_ARRAYSIZE(kVulkanLeaves));
        if ((engineMatch || vulkanMatch) &&
            ImGui::CollapsingHeader("Engine", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent();
            drawLeaves(kEngineLeaves, IM_ARRAYSIZE(kEngineLeaves));
            if (vulkanMatch && ImGui::TreeNodeEx("Vulkan", ImGuiTreeNodeFlags_DefaultOpen)) {
                drawLeaves(kVulkanLeaves, IM_ARRAYSIZE(kVulkanLeaves));
                ImGui::TreePop();
            }
            ImGui::Unindent();
        }

        ImGui::EndChild();

        ImGui::SameLine(0.0f, 8.0f);

        ImGui::BeginChild("##settingsContent", ImVec2(0.0f, 0.0f), true);
        bool dirty = false;
        switch (m_selectedCategory) {
            case 0:  dirty = drawOutline(target);              break;
            case 1:  dirty = drawCamera(effective, target);    break;
            case 2:  dirty = drawGizmo(effective, target);     break;
            case 3:  dirty = drawContentBrowser(effective, target); break;
            case 4:  dirty = drawEditorUi(effective, target);  break;
            case 5:  dirty = drawRendering(effective, target); break;
            case 6:  dirty = drawWindow(effective, target);    break;
            case 7:  dirty = drawLogging(effective, target);   break;
            case 8:  dirty = drawSwapchain(effective, target); break;
            case 9:  dirty = drawDevice(effective, target);    break;
            case 10: dirty = drawTextures(effective, target);  break;
            case 11: dirty = drawShaders(effective, target);   break;
            case 12: dirty = drawPipeline(effective, target);  break;
            default: break;
        }
        ImGui::EndChild();

        if (dirty) {
            // Piggyback the active category onto a save that is happening anyway, so the
            // window reopens on the same section next time without writing on every click.
            if (readAt(target, categoryKey).getAs<int>().value_or(-1) != m_selectedCategory)
                writeAt(target, categoryKey) = static_cast<double>(m_selectedCategory);

            SettingsManager::markChanged();
            if (const auto [result, message] = Application::getSettingsManager().save(scope); result != SUCCESS)
                VC_ERROR_NO_EXIT("Could not save settings: {}", message);
        }

        ImGui::End();
    }
} // Vectrix
