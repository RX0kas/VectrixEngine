#ifndef VECTRIXWORKSPACE_IMGUIMANAGER_H
#define VECTRIXWORKSPACE_IMGUIMANAGER_H

#include "Vectrix/Core/Window.h"

namespace Vectrix {
    class ImGuiManager {
    public:
        virtual ~ImGuiManager() = default;

        virtual void initImGui() = 0;
        virtual void render() = 0;
        virtual void update() = 0;
        virtual void cleanup() = 0;
        virtual void attachDebugGraphicWidget() = 0;
        static ImGuiManager* create(Window& window);
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_IMGUIMANAGER_H