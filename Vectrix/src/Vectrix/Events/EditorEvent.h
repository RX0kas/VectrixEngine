#ifndef VECTRIXWORKSPACE_EDITOREVENT_H
#define VECTRIXWORKSPACE_EDITOREVENT_H
#include <string>
#include <ostream>

#include "Event.h"

namespace Vectrix {
    class EditorEvent : public Event {
    public:
        EVENT_CLASS_CATEGORY(EventCategoryEditor)
        [[nodiscard]] std::string toString() const override { return getName(); }
    protected:
        EditorEvent() = default;
    };

    class SceneSavedEvent : public EditorEvent {
    public:
        SceneSavedEvent(const std::string &name) : EditorEvent(), m_name(name) {}

        [[nodiscard]] std::string toString() const override	{
            return "SceneSavedEvent: " + m_name;
        }

        EVENT_CLASS_TYPE(SceneSaved)
    private:
        std::string m_name;
    };

    class SceneLoadingEvent : public EditorEvent {
    public:
        SceneLoadingEvent(const std::string &name) : EditorEvent(), m_name(name) {}

        [[nodiscard]] std::string toString() const override	{
            return "SceneLoadingEvent: " + m_name;
        }

        EVENT_CLASS_TYPE(SceneLoading)
    private:
        std::string m_name;
    };

    class SceneLoadedEvent : public EditorEvent {
    public:
        SceneLoadedEvent(const std::string &name) : EditorEvent(), m_name(name) {}

        [[nodiscard]] std::string toString() const override	{
            return "SceneLoadedEvent: " + m_name;
        }

        EVENT_CLASS_TYPE(SceneLoaded)
    private:
        std::string m_name;
    };
}
#endif //VECTRIXWORKSPACE_EDITOREVENT_H