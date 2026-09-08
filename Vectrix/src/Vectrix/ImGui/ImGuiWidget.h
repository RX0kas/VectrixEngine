#ifndef VECTRIXWORKSPACE_IMGUIWIDGET_H
#define VECTRIXWORKSPACE_IMGUIWIDGET_H

#include "vcpch.h"

/**
 * @file ImGuiWidget.h
 * @brief Definition of the ImGuiWidget
 * @ingroup imgui
 */

namespace Vectrix {
    /**
     * @brief This function represent an ImGui widget
     *
     * Derive from it and put the ImGui calls in render, then hand the widget to
     * ImGuiLayer::addWidget so it is drawn every frame while it is enabled.
     * @see ImGuiLayer::addWidget
     * @ingroup imgui
     */
    class ImGuiWidget {
    public:
        /**
         * @brief This function create a new widget
         * @param name The name of the widget
         */
        ImGuiWidget(std::string name = "Widget");
        virtual ~ImGuiWidget() = 0;

        /**
         * @brief This function is what will be called every frame
         */
        virtual void render() = 0;

        /**
         * @brief This function return the name of the widget
         */
        [[nodiscard]] const std::string& getName() const { return m_DebugName; }

        /**
         * @brief Tell if the widget is currently drawn
         * @return true when the widget is enabled
         */
        [[nodiscard]] bool isEnable() const { return m_enable; }

        /**
         * @brief Choose whether the widget is drawn
         * @param enable true to draw the widget, false to skip it
         */
        void setEnable(bool enable) { m_enable = enable; }

        /**
         * @brief Stop drawing the widget
         * @see enable
         */
        void disable() { m_enable = false; }

        /**
         * @brief Start drawing the widget again
         * @see disable
         */
        void enable() { m_enable = true; }

        /**
         * @brief Return the enabled flag itself, so it can be bound to a widget
         * @return A reference to the flag, for example to pass to an ImGui checkbox
         */
        bool& getEnable() { return m_enable; }
    protected:
        std::string m_DebugName;
        bool m_enable = true;
        /// Whether a newly created widget starts enabled
        static bool vc_defaultEnable;
    };
}
#endif //VECTRIXWORKSPACE_IMGUIWIDGET_H