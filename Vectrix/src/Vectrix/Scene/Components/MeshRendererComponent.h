#ifndef VECTRIXWORKSPACE_MESHCOMPONENT_H
#define VECTRIXWORKSPACE_MESHCOMPONENT_H
#include "Vectrix/Physics/AABB.h"
#include "Vectrix/Rendering/Buffer.h"
#include "Vectrix/Rendering/Mesh/Vertex.h"
#include "Vectrix/Rendering/Mesh/VertexArray.h"
#include "Vectrix/Rendering/Shaders/Shader.h"

namespace Vectrix {
    class MeshRendererComponent {
    public:
        std::shared_ptr<VertexArray> vertexArray;
        std::shared_ptr<Texture> texture;
        BufferLayout layout;
        std::shared_ptr<Shader> shader;
        AABB aabb;

        MeshRendererComponent(const MeshRendererComponent&) = delete;
        MeshRendererComponent(Entity* entity);
        MeshRendererComponent(Entity* entity,const std::vector<Vertex>& vertices, std::shared_ptr<Shader> shader, const std::shared_ptr<Texture>& texture);
        MeshRendererComponent(Entity* entity,const std::string &pathObj, std::shared_ptr<Shader> shader, const std::shared_ptr<Texture>& texture);
        MeshRendererComponent(Entity* entity,const std::vector<Vertex>& vertices, std::vector<uint32_t> indices, std::shared_ptr<Shader> shader, const std::shared_ptr<Texture>& texture);

        /**
         * @brief Return true if this mesh renderer can be enabled
         */
        [[nodiscard]] bool canEnable() const { return shader!=nullptr && texture!=nullptr && vertexArray != nullptr; }

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
        void registerMesh();
        friend class VulkanContext;
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
        bool enable = true;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_MESHCOMPONENT_H