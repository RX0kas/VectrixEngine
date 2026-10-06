#ifndef VECTRIXWORKSPACE_EDITOREVENTS_H
#define VECTRIXWORKSPACE_EDITOREVENTS_H
#include <filesystem>
#include <memory>
#include <string>

#include "Vectrix/Events/Event.h"
#include "Vectrix/Scene/Component.h"
#include "Vectrix/Scene/Entity.h"
#include "Vectrix/Scene/Scene.h"
#include "Vectrix/Utils/Path.h"

// The editor's own events, sent through Application like the engine's (see EventBase)
namespace Vectrix {
    /// Sent (sendEvent, right away) by the content browser once it moved or renamed a file or folder of the
    /// assets folder. EditorLayer updates the loaded assets and the project's scenes on it
    class AssetMovedEvent : public EventBase<AssetMovedEvent, "AssetMoved"> {
    public:
        AssetMovedEvent(std::filesystem::path from, std::filesystem::path to) : from(std::move(from)), to(std::move(to)) {}

        std::filesystem::path from; ///< Where it was
        std::filesystem::path to;   ///< Where it is now

        [[nodiscard]] std::string toString() const override {
            return std::format("AssetMoved: {} -> {}", toUtf8(from), toUtf8(to));
        }
    };

    /// Posted once a scene replaced the open one: opened from a file, or a new empty one
    class SceneOpenedEvent : public EventBase<SceneOpenedEvent, "SceneOpened"> {
    public:
        SceneOpenedEvent(std::shared_ptr<Scene> scene, std::filesystem::path path) : scene(std::move(scene)), path(std::move(path)) {}

        std::shared_ptr<Scene> scene; ///< The scene now open
        std::filesystem::path path;   ///< Its file, empty for a new scene not saved yet

        [[nodiscard]] std::string toString() const override {
            return std::format("SceneOpened: {}", path.empty() ? "new scene" : toUtf8(path));
        }
    };

    /// Posted when the selected entity changed, from the hierarchy, the viewport or an undo/redo
    class SelectionChangedEvent : public EventBase<SelectionChangedEvent, "SelectionChanged"> {
    public:
        explicit SelectionChangedEvent(std::shared_ptr<Entity> entity)
            : entity(std::move(entity)), name(this->entity ? this->entity->getComponent<InformationComponent>().name : "") {}

        /// The entity now selected, null when none is. Posted: it may have been deleted by the time it is received
        std::shared_ptr<Entity> entity;
        std::string name; ///< Its name when it was selected

        [[nodiscard]] std::string toString() const override {
            return entity ? std::format("SelectionChanged: {}", name) : std::string("SelectionChanged: none");
        }
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_EDITOREVENTS_H
