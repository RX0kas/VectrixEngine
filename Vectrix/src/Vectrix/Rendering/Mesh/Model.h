#ifndef VECTRIXWORKSPACE_MODEL_H
#define VECTRIXWORKSPACE_MODEL_H
#include "Transform.h"
#include "VertexArray.h"

namespace Vectrix {
    struct MeshHandle {
        uint32_t firstIndex;
        uint32_t indexCount;
        int32_t  vertexOffset;
    };

    class Model {
    public:
        [[nodiscard]] Ref<VertexArray> getVertexArray() const { return m_vertexArray; }
        [[nodiscard]] BufferLayout getLayout() const { return m_layout; }

        void setPosition(glm::vec3 position) { m_transform.translation=position; }
        void setRotation(glm::vec3 rotation) { m_transform.rotation=rotation; }
        void setScale(glm::vec3 scale) { m_transform.scale=scale; }
        [[nodiscard]] glm::vec3 getPosition() const { return m_transform.translation; }
        [[nodiscard]] glm::vec3 getRotation() const { return m_transform.rotation; }
        [[nodiscard]] glm::vec3 getScale() const { return m_transform.scale; }
        [[nodiscard]] glm::mat4 getModelMatrix() { return m_transform.mat4(); }
        [[nodiscard]] Transform getTransform() const {return m_transform; }
        [[nodiscard]] std::vector<Vertex> getVertices() const { return m_vertices; }
        [[nodiscard]] std::vector<uint32_t> getIndices() const { return m_indices; }
    private:
        friend class MeshManager;
        static Model create(const std::vector<Vertex>& vertices, const BufferLayout& layout);
        static Model create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const BufferLayout& layout);
        static Model load(const std::string& pathObj);
        Model(const std::vector<Vertex>& vertices, const BufferLayout& layout);
        Model(const std::vector<Vertex>& vertices,std::vector<uint32_t> indices, const BufferLayout& layout);
        Ref<VertexArray> m_vertexArray;
        BufferLayout m_layout;
        Transform m_transform;
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_MODEL_H