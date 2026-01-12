#include "Model.h"

#include "ObjLoader.h"

namespace Vectrix {
    Model::Model(const std::vector<Vertex>& vertices, const BufferLayout& layout) {
        auto vertexBuffer =  std::shared_ptr<VertexBuffer>(VertexBuffer::create(vertices, vertices.size()));

        vertexBuffer->setLayout(layout);

        m_vertexArray = std::unique_ptr<VertexArray>(VertexArray::create());
        m_vertexArray->addVertexBuffer(vertexBuffer);

        m_transform = {glm::vec3(0.0f),glm::vec3(1.0f),glm::vec3(0.0f)};
    }

    Model::Model(const std::vector<Vertex>& vertices,std::vector<uint32_t> indices, const BufferLayout& layout) {
        std::shared_ptr<VertexBuffer> vertexBuffer = std::shared_ptr<VertexBuffer>(VertexBuffer::create(vertices, static_cast<uint32_t>(vertices.size())));
        std::shared_ptr<IndexBuffer> indexBuffer = std::shared_ptr<IndexBuffer>(IndexBuffer::create(indices.data(),static_cast<uint32_t>(indices.size())));
        vertexBuffer->setLayout(layout);

        m_vertexArray = std::unique_ptr<VertexArray>(VertexArray::create());
        m_vertexArray->addVertexBuffer(vertexBuffer);
        m_vertexArray->setIndexBuffer(indexBuffer);

        m_layout = layout;

        m_transform = {glm::vec3(0.0f),glm::vec3(1.0f),glm::vec3(0.0f)};
    }

    Model Model::create(const std::vector<Vertex>& vertices, const BufferLayout& layout) {
        return {vertices,layout};
    }

    Model Model::create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const BufferLayout& layout) {
        return {vertices,indices,layout};
    }

    Model Model::load(const std::string &pathObj) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        if (!loadOBJ(pathObj, vertices, indices)) {
            VC_CORE_ERROR("Failed to load OBJ");
        }

        return create(vertices, indices, getTinyObjLayout());
    }

} // Vectrix