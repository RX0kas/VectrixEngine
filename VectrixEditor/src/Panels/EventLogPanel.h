#ifndef VECTRIXWORKSPACE_EVENTLOGPANEL_H
#define VECTRIXWORKSPACE_EVENTLOGPANEL_H
#include <deque>
#include <string>
#include <string_view>
#include <vector>

#include "Vectrix/Events/EventListener.h"
#include "Vectrix/ImGui/ImGuiWidget.h"

namespace Vectrix {
    /**
     * @brief Lists the last events sent, the engine's and the editor's, and who consumed each
     *
     * It observes the dispatch (Application::observeEvents), so it also shows the events ImGui or a layer
     * consumed. Nothing is recorded while the panel is closed or paused.
     */
    class EventLogPanel : public ImGuiWidget {
    public:
        EventLogPanel();
        void render() override;

    private:
        struct Entry {
            int frame;
            EventCategory categories;
            std::string text;
            std::string consumedBy;
        };

        void record(const Event& event, std::string_view consumedBy);
        [[nodiscard]] bool isShown(const Entry& entry, std::string_view search) const;

        static constexpr size_t k_maxEntries = 2000;

        ScopedSubscription m_observer;
        std::deque<Entry> m_entries;
        std::vector<size_t> m_visible; ///< The entries the filters let through, rebuilt every frame
        bool m_paused = false;
        bool m_autoScroll = true;
        bool m_recordMouseMoves = false; ///< Off by default: they flood the log
        bool m_showWindow = true, m_showKeyboard = true, m_showMouse = true, m_showOther = true;
        char m_search[64] = {};
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_EVENTLOGPANEL_H
