#ifndef VECTRIXWORKSPACE_EDITOREVENT_H
#define VECTRIXWORKSPACE_EDITOREVENT_H
#include <string>
#include <ostream>

#include "Event.h"

/**
 * @file EditorEvent.h
 * @brief Definition of the events reporting what the editor is doing to a scene
 * @ingroup events
 */

namespace Vectrix {
    /**
     * @brief Base class of every event sent by the editor
     *
     * Unlike the window and input events, these are not produced by the platform but by
     * the editor itself when it acts on a scene.
     * @ingroup events
     */
    class EditorEvent : public Event {
    public:
        EVENT_CLASS_CATEGORY(EventCategoryEditor)
        /// @copydoc Event::toString
        [[nodiscard]] std::string toString() const override { return getName(); }
    protected:
        EditorEvent() = default;
    };

    /**
     * @brief Sent once a scene has been written to disk
     * @ingroup events
     */
    class SceneSavedEvent : public EditorEvent {
    public:
        /**
         * @brief Build the event for the scene that was saved
         * @param name The name of the scene
         */
        SceneSavedEvent(const std::string &name) : EditorEvent(), m_name(name) {}

        /// @copydoc Event::toString
        [[nodiscard]] std::string toString() const override	{
            return "SceneSavedEvent: " + m_name;
        }

        EVENT_CLASS_TYPE(SceneSaved)
    private:
        std::string m_name;
    };

    /**
     * @brief Sent when a scene starts loading, before its content is available
     * @see SceneLoadedEvent
     * @ingroup events
     */
    class SceneLoadingEvent : public EditorEvent {
    public:
        /**
         * @brief Build the event for the scene being loaded
         * @param name The name of the scene
         */
        SceneLoadingEvent(const std::string &name) : EditorEvent(), m_name(name) {}

        /// @copydoc Event::toString
        [[nodiscard]] std::string toString() const override	{
            return "SceneLoadingEvent: " + m_name;
        }

        EVENT_CLASS_TYPE(SceneLoading)
    private:
        std::string m_name;
    };

    /**
     * @brief Sent once a scene finished loading and can be used
     * @see SceneLoadingEvent
     * @ingroup events
     */
    class SceneLoadedEvent : public EditorEvent {
    public:
        /**
         * @brief Build the event for the scene that finished loading
         * @param name The name of the scene
         */
        SceneLoadedEvent(const std::string &name) : EditorEvent(), m_name(name) {}

        /// @copydoc Event::toString
        [[nodiscard]] std::string toString() const override	{
            return "SceneLoadedEvent: " + m_name;
        }

        EVENT_CLASS_TYPE(SceneLoaded)
    private:
        std::string m_name;
    };
}
#endif //VECTRIXWORKSPACE_EDITOREVENT_H
