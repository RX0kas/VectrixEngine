#include "Mesh.h"

#include "ObjLoader.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Rendering/Renderer.h"

namespace Vectrix {
    Mesh::Mesh(const std::vector<Vertex> &vertices, std::vector<uint32_t> indices) : m_aabb(vertices) {
        VC_PROFILER_FUNCTION();
        m_layout = getTinyObjLayout();
        const auto vertexBuffer = VertexBuffer::create(vertices, static_cast<uint32_t>(vertices.size()));
        const auto indexBuffer = IndexBuffer::create(indices.data(),static_cast<uint32_t>(indices.size()));
        vertexBuffer->setLayout(m_layout);

        m_vertexArray = VertexArray::create();
        m_vertexArray->addVertexBuffer(vertexBuffer);
        m_vertexArray->setIndexBuffer(indexBuffer);
        m_vertices = vertices;
        m_indices = indices;
    }

    Mesh::Mesh(const std::vector<Vertex> &vertices) : m_aabb(vertices) {
        VC_PROFILER_FUNCTION();
        m_layout = getTinyObjLayout();
        const auto vertexBuffer = std::shared_ptr<VertexBuffer>(VertexBuffer::create(vertices, vertices.size()));
        m_vertices = vertices;

        vertexBuffer->setLayout(m_layout);

        m_vertexArray = VertexArray::create();
        m_vertexArray->addVertexBuffer(vertexBuffer);
    }

    void Mesh::registerMesh() {
        switch (Renderer::getAPI()) {
            case RendererAPI::API::Vulkan:
                VulkanContext::instance().registerMesh(this);
                return;
            case RendererAPI::API::None:
                VC_CORE_ERROR("Can't register Mesh because RendererAPI is None");
            default:
                VC_CORE_ERROR("Unknown RendererAPI");
        }
    }
} // Vectrix