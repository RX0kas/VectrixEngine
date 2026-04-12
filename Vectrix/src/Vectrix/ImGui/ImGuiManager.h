#ifndef VECTRIXWORKSPACE_IMGUIMANAGER_H
#define VECTRIXWORKSPACE_IMGUIMANAGER_H
/// @cond INTERNAL
namespace Vectrix {
    class Window;
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
///@endcond
#endif //VECTRIXWORKSPACE_IMGUIMANAGER_H