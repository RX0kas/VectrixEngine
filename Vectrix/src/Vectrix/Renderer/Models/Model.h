#ifndef VECTRIXWORKSPACE_MODEL_H
#define VECTRIXWORKSPACE_MODEL_H
#include "Transform.h"
#include "VertexArray.h"

namespace Vectrix {
    class Model {
    public:
        static Model create(const std::vector<Vertex>& vertices, const BufferLayout& layout);
        static Model create(const std::string &filepath, const BufferLayout& layout);
        static Model create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const BufferLayout& layout);
        static Model load(const std::string& pathObj);


        [[nodiscard]] VertexArray* getVertexArray() const {return m_vertexArray.get();}
        [[nodiscard]] BufferLayout getLayout() const {return m_layout;}

        void setPosition(glm::vec3 position) {m_transform.translation=position;}
        void setRotation(glm::vec3 rotation) {m_transform.rotation=rotation;}
        void setScale(glm::vec3 scale) {m_transform.scale=scale;}
        [[nodiscard]] glm::vec3 getPosition() const {return m_transform.translation;}
        [[nodiscard]] glm::vec3 getRotation() const {return m_transform.rotation;}
        [[nodiscard]] glm::vec3 getScale() const {return m_transform.scale;}
        [[nodiscard]] glm::mat4 getModelMatrix() {return m_transform.mat4();}
        [[nodiscard]] Transform getTransform() const {return m_transform;}
    private:
        Model(const std::vector<Vertex>& vertices, const BufferLayout& layout);
        Model(const std::vector<Vertex>& vertices,std::vector<uint32_t> indices, const BufferLayout& layout);
        Ref<VertexArray> m_vertexArray;
        BufferLayout m_layout;
        // TODO: Maybe put it somewhere else
        Transform m_transform;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_MODEL_H