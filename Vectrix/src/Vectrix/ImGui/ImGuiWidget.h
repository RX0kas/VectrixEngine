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
    protected:
        std::string m_DebugName;
    };
}
#endif //VECTRIXWORKSPACE_IMGUIWIDGET_H