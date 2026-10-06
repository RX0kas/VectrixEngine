#include "EventLogPanel.h"

#include <algorithm>
#include <cctype>
#include <cfloat>

#include "imgui.h"
#include "Vectrix/Application.h"

namespace Vectrix {
    namespace {
        std::string toLower(std::string_view text) {
            std::string out(text);
            std::ranges::transform(out, out.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return out;
        }

        bool hasCategory(EventCategory categories, EventCategory category) {
            return (categories & category) != EventCategory::None;
        }
    }

    EventLogPanel::EventLogPanel() : ImGuiWidget("Event Log") {
        m_observer = Application::instance().observeEvents([this](const Event& event, std::string_view consumedBy) {
            record(event, consumedBy);
        });
    }

    void EventLogPanel::record(const Event& event, std::string_view consumedBy) {
        if (!isEnable() || m_paused)
            return;
        if (!m_recordMouseMoves && event.is<MouseMovedEvent>())
            return;
        if (m_entries.size() == k_maxEntries)
            m_entries.pop_front();
        m_entries.push_back({ImGui::GetFrameCount(), event.getCategories(), event.toString(), std::string(consumedBy)});
    }

    bool EventLogPanel::isShown(const Entry& entry, std::string_view search) const {
        const bool window = hasCategory(entry.categories, EventCategory::Window);
        const bool keyboard = hasCategory(entry.categories, EventCategory::Keyboard);
        const bool mouse = hasCategory(entry.categories, EventCategory::Mouse);
        const bool other = !window && !keyboard && !mouse; // the editor's own events
        if (!(m_showWindow && window) && !(m_showKeyboard && keyboard) && !(m_showMouse && mouse) && !(m_showOther && other))
            return false;
        return search.empty() || toLower(entry.text).find(search) != std::string::npos
            || toLower(entry.consumedBy).find(search) != std::string::npos;
    }

    void EventLogPanel::render() {
        if (!ImGui::Begin(getName().c_str(), &getEnable())) {
            ImGui::End();
            return;
        }

        if (ImGui::Button(m_paused ? "Resume" : "Pause", ImVec2(70, 0)))
            m_paused = !m_paused;
        ImGui::SameLine();
        if (ImGui::Button("Clear"))
            m_entries.clear();
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &m_autoScroll);
        ImGui::SameLine();
        ImGui::Checkbox("Record mouse moves", &m_recordMouseMoves);

        ImGui::Checkbox("Window", &m_showWindow);
        ImGui::SameLine();
        ImGui::Checkbox("Keyboard", &m_showKeyboard);
        ImGui::SameLine();
        ImGui::Checkbox("Mouse", &m_showMouse);
        ImGui::SameLine();
        ImGui::Checkbox("Editor", &m_showOther);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("The events without an engine category: the editor's own (AssetMoved, SceneOpened...)");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputTextWithHint("##search", "Search", m_search, sizeof(m_search));

        const std::string search = toLower(m_search);
        m_visible.clear();
        for (size_t i = 0; i < m_entries.size(); ++i)
            if (isShown(m_entries[i], search))
                m_visible.push_back(i);
        ImGui::TextDisabled("%zu shown, %zu recorded (the last %zu are kept)", m_visible.size(), m_entries.size(), k_maxEntries);

        constexpr ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY
            | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;
        if (ImGui::BeginTable("##events", 3, flags)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Frame", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Event", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Consumed by", ImGuiTableColumnFlags_WidthFixed, 110.0f);
            ImGui::TableHeadersRow();

            ImGuiListClipper clipper;
            clipper.Begin(static_cast<int>(m_visible.size()));
            while (clipper.Step()) {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                    const Entry& entry = m_entries[m_visible[static_cast<size_t>(row)]];
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextDisabled("%d", entry.frame);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(entry.text.c_str());
                    ImGui::TableNextColumn();
                    if (entry.consumedBy.empty())
                        ImGui::TextDisabled("-");
                    else
                        ImGui::TextColored(ImVec4(0.45f, 0.62f, 0.90f, 1.0f), "%s", entry.consumedBy.c_str());
                }
            }
            // Follows the new events while scrolled to the bottom
            if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
            ImGui::EndTable();
        }
        ImGui::End();
    }
} // Vectrix
