#ifndef VECTRIXWORKSPACE_MESHCOMPONENT_H
#define VECTRIXWORKSPACE_MESHCOMPONENT_H
#include "Vectrix/Rendering/Mesh/Mesh.h"
#include "Vectrix/Rendering/Shaders/Shader.h"

/**
 * @file MeshRendererComponent.h
 * @brief Definition of the component that makes an entity visible
 * @ingroup ecs
 */

namespace Vectrix {
    /**
     * @brief Gives an entity the geometry, the material and the texture needed to draw it
     *
     * The three pieces are set independently, which means the component can exist while
     * still being incomplete. It only draws once all three are there and it is enabled,
     * so use tryEnabling rather than assuming it is ready.
     * @see Mesh
     * @see Shader
     * @see Texture
     * @ingroup ecs
     */
    class MeshRendererComponent {
    public:
        /// The geometry to draw
        std::shared_ptr<Mesh> mesh;

        /// The texture applied to the geometry
        std::shared_ptr<Texture> texture;

        /// The shader the geometry is drawn with
        std::shared_ptr<Shader> shader;


        MeshRendererComponent(const MeshRendererComponent&) = delete;

        /**
         * @brief Build an empty renderer, with no mesh, shader nor texture yet
         * @param entity The entity the component is added to
         * @note It cannot draw until all three are set
         * @see tryEnabling
         */
        MeshRendererComponent(std::shared_ptr<Entity> entity);

        /**
         * @brief Build a renderer that is ready to draw
         * @param entity The entity the component is added to
         * @param mesh The geometry to draw
         * @param shader The shader to draw it with
         * @param texture The texture to apply
         */
        MeshRendererComponent(std::shared_ptr<Entity> entity, const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Shader> &shader, const std::shared_ptr<Texture>& texture);

        /**
         * @brief Return true if this mesh renderer can be enabled
         * @return true once the mesh, the shader and the texture are all set
         */
        [[nodiscard]] bool canEnable() const { return shader!=nullptr && texture!=nullptr && mesh != nullptr; }

        /**
         * @brief Try enabling this mesh renderer and return true if it succeeded
         * @return true when it was ready and is now enabled
         * @see canEnable
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
         * @return true when the entity is drawn
         */
        [[nodiscard]] bool isEnable() const { return enable; }

        /**
         * @brief Disable the meshRenderer
         * @post The entity stops being drawn until tryEnabling succeeds again
         */
        void disable() { enable = false; }
    private:
        bool enable = true;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_MESHCOMPONENT_H