#include "SceneHierarchyPanel.h"

#include <filesystem>

#include "imgui.h"
#include "Vectrix/Scene/Component.h"
#include <glm/gtc/type_ptr.hpp>

#include "Vectrix/Assets/AssetsManager.h"
#include "Undo/Commands.h"
#include "Utils/Error.h"

namespace Vectrix {
    SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene> &scene) : ImGuiWidget("SceneHierarchyPanel") {
        setContext(scene);
    }

    void SceneHierarchyPanel::setContext(const std::shared_ptr<Scene> &scene) {
        m_context = scene;
        resetSelectedEntity();
    }

    void SceneHierarchyPanel::pushAndRefreshSelection(std::unique_ptr<Command> command) {
        if (!m_undoHistory) return;
        Command* raw = m_undoHistory->push(std::move(command));
        if (auto* entityHandle = dynamic_cast<EntityHandleCommand*>(raw))
            m_selectionContext = entityHandle->currentHandle();
    }

    void SceneHierarchyPanel::deleteSelectedEntity() {
        if (!m_selectionContext) return;
        pushAndRefreshSelection(std::make_unique<DeleteEntityCommand>(m_context, m_selectionContext));
    }

    void SceneHierarchyPanel::render() {
        ImGui::Begin("Scene Hierarchy");
        for (const auto& e : m_context->m_entities) {
            const std::shared_ptr<Entity>& entity = e.second;
            drawEntityNode(entity);
        }
        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
            m_selectionContext = {};

        if (ImGui::IsWindowFocused() && m_selectionContext && ImGui::IsKeyPressed(ImGuiKey_Delete))
            deleteSelectedEntity();

        if (ImGui::BeginPopupContextWindow(nullptr, 1)) {
            if (ImGui::MenuItem("Create Empty Entity"))
                pushAndRefreshSelection(std::make_unique<CreateEntityCommand>(m_context, "Empty Entity"));

            ImGui::EndPopup();
        }

        ImGui::End();

        ImGui::Begin("Properties");
        if (m_selectionContext) {
            drawProperties(m_selectionContext);

            if (ImGui::Button("Add Component"))
                ImGui::OpenPopup("AddComponent");
        }

        if (ImGui::BeginPopup("AddComponent")) {
            bool hasOneComponent = false;
            if (!m_selectionContext->hasComponent<CameraComponent>()) {
                hasOneComponent = true;
                if (ImGui::MenuItem("Camera")) {
                    pushAndRefreshSelection(std::make_unique<AddComponentCommand<CameraComponent>>(m_selectionContext));
                    ImGui::CloseCurrentPopup();
                }
            }

            if (!m_selectionContext->hasComponent<MeshRendererComponent>()) {
                hasOneComponent = true;
                if (ImGui::MenuItem("MeshRenderer")) {
                    pushAndRefreshSelection(std::make_unique<AddComponentCommand<MeshRendererComponent>>(m_selectionContext));
                    ImGui::CloseCurrentPopup();
                }
            }

            if (!hasOneComponent) {
                ImGui::Text("No component can be added");
            }

            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void SceneHierarchyPanel::drawEntityNode(const std::shared_ptr<Entity>& entity) {
        std::string name = entity->getComponent<InformationComponent>().name;

        if (ImGui::Selectable(name.empty() ? "##" : name.c_str(), m_selectionContext && entity->getID()==m_selectionContext->getID())) {
            m_selectionContext = entity;
        }

        if (ImGui::BeginPopupContextItem()) {
            m_selectionContext = entity;
            if (ImGui::MenuItem("Delete Entity"))
                deleteSelectedEntity();
            ImGui::EndPopup();
        }
    }

    template<typename T>
    static bool drawAssetDropField(const char* label,std::shared_ptr<T>& asset, const char* payloadType, const char* emptyText, MeshRendererComponent& mc) {
        bool changed = false;

        ImGui::PushID(label);

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        float fullWidth = ImGui::GetContentRegionAvail().x;
        float height = 58.0f;
        float rounding = 8.0f;
        float spacingY = 8.0f;

        ImVec2 start = ImGui::GetCursorScreenPos();
        ImVec2 size(fullWidth, height);
        ImVec2 end(start.x + size.x, start.y + size.y);

        float clearButtonSize = 24.0f;
        float clearButtonPadding = 10.0f;
        float reservedRight = asset ? clearButtonSize + clearButtonPadding * 2.0f : clearButtonPadding;

        ImGui::InvisibleButton("##DropZone",ImVec2(fullWidth - reservedRight, height));

        bool hovered = ImGui::IsItemHovered();

        bool payloadHovered = false;

        if (ImGui::BeginDragDropTarget()) {
            payloadHovered = true;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadType)) {
#ifdef VC_PLATFORM_LINUX
                const char* path = static_cast<const char*>(payload->Data);
                std::filesystem::path assetPath = path;
#else
                const wchar_t* path = static_cast<const wchar_t*>(payload->Data);
                std::filesystem::path assetPath = path;
#endif

                auto [result, loadedAsset] = AssetsManager::load<T>(assetPath);

                if (result == SUCCESS && loadedAsset) {
                    asset = loadedAsset;
                    changed = true;
                } else {
                    VC_ERROR_NO_EXIT("Failed to load asset dropped on {}: {} ({})",label,assetPath.string(),toString(result));
                }
            }

            ImGui::EndDragDropTarget();
        }

        ImU32 bgColor = ImGui::GetColorU32(hovered || payloadHovered ? ImVec4(0.18f, 0.185f, 0.20f, 1.0f) : ImVec4(0.13f, 0.135f, 0.15f, 1.0f));
        ImU32 borderColor = ImGui::GetColorU32(ImVec4(0.45f, 0.62f, 0.90f, 1.0f));

        drawList->AddRectFilled(start, end, bgColor, rounding);
        drawList->AddRect(start, end, borderColor, rounding, 0, hovered ? 2.0f : 1.0f);

        ImVec2 iconMin(start.x + 10.0f, start.y + 11.0f);
        ImVec2 iconMax(start.x + 46.0f, start.y + 47.0f);

        drawList->AddRectFilled(iconMin,iconMax,ImGui::GetColorU32(ImVec4(0.20f, 0.205f, 0.23f, 1.0f)),6.0f);

        const char* glyph = asset ? "A" : "+";
        ImVec2 glyphSize = ImGui::CalcTextSize(glyph);

        drawList->AddText(
            ImVec2(iconMin.x + ((iconMax.x - iconMin.x) - glyphSize.x) * 0.5f,iconMin.y + ((iconMax.y - iconMin.y) - glyphSize.y) * 0.5f),
            ImGui::GetColorU32(asset ? ImVec4(0.70f, 0.82f, 1.0f, 1.0f) : ImVec4(0.55f, 0.57f, 0.62f, 1.0f)),
            glyph
        );

        const std::string valueText = asset ? asset->getID() : emptyText;

        const float textStartX = start.x + 58.0f;
        const float textMaxX = end.x - reservedRight - 6.0f;
        const float textWidth = std::max(20.0f, textMaxX - textStartX);

        drawList->AddText(ImVec2(textStartX, start.y + 10.0f),ImGui::GetColorU32(ImVec4(0.78f, 0.80f, 0.86f, 1.0f)),label);

        const std::string& displayValue = valueText;
        ImGui::SetCursorScreenPos(ImVec2(textStartX, start.y + 31.0f));
        ImGui::PushTextWrapPos(textStartX + textWidth);
        ImGui::TextColored(asset ? ImVec4(0.95f, 0.95f, 0.98f, 1.0f) : ImVec4(0.50f, 0.52f, 0.58f, 1.0f),"%s",displayValue.c_str());
        ImGui::PopTextWrapPos();

        if (asset) {
            ImVec2 buttonPos(end.x - clearButtonSize - clearButtonPadding,start.y + (height - clearButtonSize) * 0.5f);

            ImGui::SetCursorScreenPos(buttonPos);

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.225f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.18f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.45f, 0.12f, 0.12f, 1.0f));

            if (ImGui::Button("x", ImVec2(clearButtonSize, clearButtonSize))) {
                mc.disable();
                asset.reset();
                changed = true;
            }

            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Clear %s", label);
            }
        }

        ImGui::SetCursorScreenPos(ImVec2(start.x, end.y + spacingY));

        if (hovered && !asset) {
            ImGui::SetTooltip("Drop a %s here", label);
        }

        ImGui::PopID();

        return changed;
    }

    void SceneHierarchyPanel::drawProperties(const std::shared_ptr<Entity>& entity) {
        // InformationComponent
        {
            auto& ic = entity->getComponent<InformationComponent>();
            char buffer[256] = {};
            strcpy(buffer,ic.name.c_str());
            bool edited = ImGui::InputText("Name",buffer,sizeof(buffer));
            if (ImGui::IsItemActivated())
                m_nameEditBefore = ic.name;
            if (edited)
                ic.name = std::string(buffer);
            if (ImGui::IsItemDeactivatedAfterEdit() && m_nameEditBefore != ic.name) {
                pushAndRefreshSelection(std::make_unique<PropertyChangeCommand<std::string>>(
                    "Rename", m_nameEditBefore, ic.name,
                    [entity](const std::string& v) { entity->getComponent<InformationComponent>().name = v; }));
            }
        }

        // TransformComponent
        {
            auto& tc = entity->getComponent<TransformComponent>();
            bool mustBeRemoved = false;
            if (drawTreeNodeComponent("Transform",mustBeRemoved,false)) {
                // Position
                float p[3] = {tc.position.x,tc.position.y,tc.position.z};
                bool posChanged = ImGui::DragFloat3("Position",p,0.1);
                if (ImGui::IsItemActivated())
                    m_transformDragBefore = {tc.position, tc.scale, tc.rotation};
                if (posChanged)
                    tc.position = {p[0],p[1],p[2]};
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    pushAndRefreshSelection(std::make_unique<TransformChangeCommand>(
                        entity, m_transformDragBefore, TransformSnapshot{tc.position, tc.scale, tc.rotation}));
                }

                // Rotation
                glm::vec3 r = tc.getRotationDeg();
                bool rotChanged = ImGui::DragFloat3("Rotation",glm::value_ptr(r),0.1);
                if (ImGui::IsItemActivated())
                    m_transformDragBefore = {tc.position, tc.scale, tc.rotation};
                if (rotChanged)
                    tc.setRotationDeg(r);
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    pushAndRefreshSelection(std::make_unique<TransformChangeCommand>(
                        entity, m_transformDragBefore, TransformSnapshot{tc.position, tc.scale, tc.rotation}));
                }

                // Scale
                float s[3] = {tc.scale.x,tc.scale.y,tc.scale.z};
                bool scaleChanged = ImGui::DragFloat3("Scale",s,0.1);
                if (ImGui::IsItemActivated())
                    m_transformDragBefore = {tc.position, tc.scale, tc.rotation};
                if (scaleChanged)
                    tc.scale = {s[0],s[1],s[2]};
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    pushAndRefreshSelection(std::make_unique<TransformChangeCommand>(
                        entity, m_transformDragBefore, TransformSnapshot{tc.position, tc.scale, tc.rotation}));
                }

                ImGui::TreePop();
            }
        }
        // MeshComponent
        if (entity->hasComponent<MeshRendererComponent>()) {
            auto& mc = entity->getComponent<MeshRendererComponent>();
            bool mustBeRemoved = false;

            if (drawTreeNodeComponent("Mesh Renderer", mustBeRemoved)) {
                bool isEnable = mc.isEnable();

                if (ImGui::Checkbox("Enable", &isEnable)) {
                    bool before = mc.isEnable();
                    if (isEnable) {
                        if (!mc.tryEnabling()) {
                            isEnable = false;
                            showErrorMessage("MeshRendererEnableError");
                        }
                    } else {
                        mc.disable();
                    }
                    if (mc.isEnable() != before) {
                        pushAndRefreshSelection(std::make_unique<PropertyChangeCommand<bool>>(
                            "Toggle Mesh Enabled", before, mc.isEnable(),
                            [entity](const bool& v) {
                                auto& c = entity->getComponent<MeshRendererComponent>();
                                if (v) c.tryEnabling(); else c.disable();
                            }));
                    }
                }

                renderErrorMessage("MeshRendererEnableError","Cannot enable Mesh Renderer", [&mc]{
                    ImGui::Text("Missing required data:");
                    if (mc.shader == nullptr)  ImGui::BulletText("Shader is not set");
                    if (mc.texture == nullptr) ImGui::BulletText("Texture is not set");
                    if (mc.mesh == nullptr) ImGui::BulletText("Mesh is not set");
                });

                {
                    auto before = mc.shader;
                    if (drawAssetDropField("Shader",mc.shader,"CONTENT_BROWSER_SHADER","Drop Shader here",mc)) {
                        pushAndRefreshSelection(std::make_unique<PropertyChangeCommand<std::shared_ptr<Shader>>>(
                            "Set Shader", before, mc.shader,
                            [entity](const std::shared_ptr<Shader>& v) { entity->getComponent<MeshRendererComponent>().shader = v; }));
                    }
                }
                {
                    auto before = mc.texture;
                    if (drawAssetDropField("Texture",mc.texture,"CONTENT_BROWSER_TEXTURE","Drop texture here", mc)) {
                        pushAndRefreshSelection(std::make_unique<PropertyChangeCommand<std::shared_ptr<Texture>>>(
                            "Set Texture", before, mc.texture,
                            [entity](const std::shared_ptr<Texture>& v) { entity->getComponent<MeshRendererComponent>().texture = v; }));
                    }
                }
                {
                    auto before = mc.mesh;
                    if (drawAssetDropField("Mesh",mc.mesh,"CONTENT_BROWSER_MESH","Drop mesh here", mc)) {
                        pushAndRefreshSelection(std::make_unique<PropertyChangeCommand<std::shared_ptr<Mesh>>>(
                            "Set Mesh", before, mc.mesh,
                            [entity](const std::shared_ptr<Mesh>& v) { entity->getComponent<MeshRendererComponent>().mesh = v; }));
                    }
                }

                ImGui::TreePop();
            }

            if (mustBeRemoved)
                pushAndRefreshSelection(std::make_unique<RemoveMeshRendererComponentCommand>(entity));
        }

        // CameraComponent
        if (entity->hasComponent<CameraComponent>()) {
            auto& cc = entity->getComponent<CameraComponent>();
            bool mustBeRemoved = false;
            if (drawTreeNodeComponent("Camera",mustBeRemoved)) {
                Camera& camera = cc.camera;
                bool changed = false;
                float fov = camera.getFOV();
                float camNear = camera.getCamNear();
                float camFar = camera.getCamFar();

                bool fovChanged = ImGui::DragFloat("FOV",&fov,0.01,0.01);
                if (ImGui::IsItemActivated()) m_floatDragBefore = camera.getFOV();
                if (fovChanged) { camera.setFOV(fov); changed = true; }
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    pushAndRefreshSelection(std::make_unique<PropertyChangeCommand<float>>(
                        "Set FOV", m_floatDragBefore, camera.getFOV(),
                        [entity](const float& v) { entity->getComponent<CameraComponent>().camera.setFOV(v); }));
                }

                bool nearChanged = ImGui::DragFloat("camNear",&camNear,0.01,0);
                if (ImGui::IsItemActivated()) m_floatDragBefore = camera.getCamNear();
                if (nearChanged) { camera.setCamNear(camNear); changed = true; }
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    pushAndRefreshSelection(std::make_unique<PropertyChangeCommand<float>>(
                        "Set Near Plane", m_floatDragBefore, camera.getCamNear(),
                        [entity](const float& v) { entity->getComponent<CameraComponent>().camera.setCamNear(v); }));
                }

                bool farChanged = ImGui::DragFloat("camFar",&camFar,1,0,INTMAX_MAX,"%.1f");
                if (ImGui::IsItemActivated()) m_floatDragBefore = camera.getCamFar();
                if (farChanged) { camera.setCamFar(camFar); changed = true; }
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    pushAndRefreshSelection(std::make_unique<PropertyChangeCommand<float>>(
                        "Set Far Plane", m_floatDragBefore, camera.getCamFar(),
                        [entity](const float& v) { entity->getComponent<CameraComponent>().camera.setCamFar(v); }));
                }

                ImGui::Separator();

                ImGui::Text("Active:");
                ImGui::SameLine(0,2);
                if (cc.camera.isCurrent())
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1), "True");
                else
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1), "False");

                if (ImGui::Button("Set as current")) {
                    pushAndRefreshSelection(std::make_unique<SetCurrentCameraCommand>(entity, Camera::getCurrentCamera()));
                }

                if (changed)
                    camera.recalculateMatrices();

                ImGui::TreePop();
            }

            if (mustBeRemoved) {
                pushAndRefreshSelection(std::make_unique<RemoveCameraComponentCommand>(entity));
            }
        }
    }

    bool SceneHierarchyPanel::drawTreeNodeComponent(const std::string& text, bool& mustBeRemoved, bool removable) {
        ImGuiTreeNodeFlags_ flags = ImGuiTreeNodeFlags_Framed;

        const float buttonWidth = ImGui::CalcTextSize("···").x + ImGui::GetStyle().FramePadding.x * 2.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });

        ImGui::SetNextItemAllowOverlap();
        bool open = ImGui::TreeNodeEx(reinterpret_cast<void *>(std::hash<std::string>{}(text)), flags, "%s",text.c_str());
        ImGui::SameLine(ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX() - buttonWidth);

        if (ImGui::Button(("···##" + text).c_str(), ImVec2{ buttonWidth, 0 })) {
            ImGui::OpenPopup(("ComponentSettings##" + text).c_str());
        }
        ImGui::PopStyleVar();

        // PopUp
        if (ImGui::BeginPopup(("ComponentSettings##" + text).c_str())) {
            if (ImGui::MenuItem("Wiki (WiP)")) {
                ImGui::CloseCurrentPopup();
            }
            if (removable) {
                if (ImGui::MenuItem("Delete Component")) {
                    mustBeRemoved = true;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }
        return open;
    }
} // Vectrix