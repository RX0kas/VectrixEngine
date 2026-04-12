#include "Model.h"

#include "ObjLoader.h"
#include "Vectrix/Debug/Profiler.h"

namespace Vectrix {
    Model::Model(const std::vector<Vertex>& vertices, const BufferLayout& layout) {
        VC_PROFILER_FUNCTION();
        const auto vertexBuffer =  std::shared_ptr<VertexBuffer>(VertexBuffer::create(vertices, vertices.size()));
        m_vertices = vertices;

        vertexBuffer->setLayout(layout);

        m_vertexArray = VertexArray::create();
        m_vertexArray->addVertexBuffer(vertexBuffer);

        m_transform = {glm::vec3(0.0f),glm::vec3(1.0f),glm::vec3(0.0f)};
    }

    Model::Model(const std::vector<Vertex>& vertices,std::vector<uint32_t> indices, const BufferLayout& layout) {
        VC_PROFILER_FUNCTION();
        const auto vertexBuffer = VertexBuffer::create(vertices, static_cast<uint32_t>(vertices.size()));
        const auto indexBuffer = IndexBuffer::create(indices.data(),static_cast<uint32_t>(indices.size()));
        vertexBuffer->setLayout(layout);

        m_vertexArray = VertexArray::create();
        m_vertexArray->addVertexBuffer(vertexBuffer);
        m_vertexArray->setIndexBuffer(indexBuffer);
        m_vertices = vertices;
        m_indices = indices;

        m_layout = layout;

        m_transform = {glm::vec3(0.0f),glm::vec3(1.0f),glm::vec3(0.0f)};
    }

    Model Model::create(const std::vector<Vertex>& vertices, const BufferLayout& layout) {
        return {vertices,layout};
    }

    /**
     * @deprecated Please use MeshManager::createModel
     * @param vertices Vertices of the model
     * @param indices Indices of the model
     * @return A new model
     */
    Model Model::create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const BufferLayout& layout) {
        return {vertices,indices,layout};
    }

    /**
     * @param pathObj Path of the mesh file
     * @return A new model
     */
    /**
    * @deprecated Please use MeshManager::loadModel
    **/
    Model Model::load(const std::string &pathObj) {
        VC_PROFILER_FUNCTION();
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        if (!loadOBJ(pathObj, vertices, indices)) {
            VC_CORE_CRITICAL("Failed to load OBJ");
        }

        return create(vertices, indices, getTinyObjLayout());
    }

} // Vectrix