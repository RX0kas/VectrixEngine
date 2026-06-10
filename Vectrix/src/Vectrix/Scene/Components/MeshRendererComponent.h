#ifndef VECTRIXWORKSPACE_MESHCOMPONENT_H
#define VECTRIXWORKSPACE_MESHCOMPONENT_H
#include "Vectrix/Rendering/Mesh/Mesh.h"
#include "Vectrix/Rendering/Shaders/Shader.h"

namespace Vectrix {
    class MeshRendererComponent {
    public:
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Texture> texture;
        std::shared_ptr<Shader> shader;


        MeshRendererComponent(const MeshRendererComponent&) = delete;
        MeshRendererComponent(Entity* entity);
        MeshRendererComponent(Entity* entity, const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Shader> &shader, const std::shared_ptr<Texture>& texture);

        /**
         * @brief Return true if this mesh renderer can be enabled
         */
        [[nodiscard]] bool canEnable() const { return shader!=nullptr && texture!=nullptr && mesh != nullptr; }

        /**
         * @brief Try enabling this mesh renderer and return true if it succeeded
         */
        bool tryEnabling() {
            if (canEnable()) {
                enable = true;
                return true;
            }
            return false;
        }

        /**
         * @brief This function return true if this meshRenderer is enabled
         */
        [[nodiscard]] bool isEnable() const { return enable; }

        /**
         * @brief Disable the meshRenderer
         */
        void disable() { enable = false; }
    private:
        bool enable = true;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_MESHCOMPONENT_H