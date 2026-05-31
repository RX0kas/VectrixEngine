#include "MeshRendererComponent.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Rendering/GraphicsContext.h"
#include "Vectrix/Rendering/Renderer.h"
#include "Vectrix/Rendering/Mesh/ObjLoader.h"
#include "Vectrix/Rendering/Mesh/VertexArray.h"

namespace Vectrix {
    MeshRendererComponent::MeshRendererComponent(Entity* entity,const std::vector<Vertex>& vertices, std::shared_ptr<Shader> shader, const std::shared_ptr<Texture>& texture) : shader(shader),texture(texture) {
        VC_PROFILER_FUNCTION();
        layout = getTinyObjLayout();
        const auto vertexBuffer = std::shared_ptr<VertexBuffer>(VertexBuffer::create(vertices, vertices.size()));
        m_vertices = vertices;

        vertexBuffer->setLayout(layout);

        vertexArray = VertexArray::create();
        vertexArray->addVertexBuffer(vertexBuffer);

        registerMesh();
    }

    MeshRendererComponent::MeshRendererComponent(Entity* entity,const std::vector<Vertex>& vertices, std::vector<uint32_t> indices, std::shared_ptr<Shader> shader, const std::shared_ptr<Texture>& texture) : shader(shader),texture(texture) {
        VC_PROFILER_FUNCTION();
        layout = getTinyObjLayout();
        const auto vertexBuffer = VertexBuffer::create(vertices, static_cast<uint32_t>(vertices.size()));
        const auto indexBuffer = IndexBuffer::create(indices.data(),static_cast<uint32_t>(indices.size()));
        vertexBuffer->setLayout(layout);

        vertexArray = VertexArray::create();
        vertexArray->addVertexBuffer(vertexBuffer);
        vertexArray->setIndexBuffer(indexBuffer);
        m_vertices = vertices;
        m_indices = indices;

        registerMesh();
    }

    MeshRendererComponent::MeshRendererComponent(Entity* entity,const std::string &pathObj, std::shared_ptr<Shader> shader, const std::shared_ptr<Texture>& texture) : shader(shader),texture(texture) {
        VC_PROFILER_FUNCTION();
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        if (!loadOBJ(pathObj, vertices, indices)) {
            VC_CORE_CRITICAL("Failed to load OBJ");
        }

        layout = getTinyObjLayout();
        const auto vertexBuffer = VertexBuffer::create(vertices, static_cast<uint32_t>(vertices.size()));
        const auto indexBuffer = IndexBuffer::create(indices.data(),static_cast<uint32_t>(indices.size()));
        vertexBuffer->setLayout(layout);

        vertexArray = VertexArray::create();
        vertexArray->addVertexBuffer(vertexBuffer);
        vertexArray->setIndexBuffer(indexBuffer);
        m_vertices = vertices;
        m_indices = indices;

        registerMesh();
        aabb = AABB(m_vertices);
    }

    MeshRendererComponent::MeshRendererComponent(Entity *entity) : enable(false) {}

    void MeshRendererComponent::registerMesh() {
        switch (Renderer::getAPI()) {
            case RendererAPI::API::Vulkan:
                VulkanContext::instance().registerMesh(this);
                return;
            case RendererAPI::API::None:
                VC_CORE_ERROR("Can't register Mesh because RendererAPI is None");
            default:
                VC_CORE_ERROR("Unkown RendererAPI");
        }
    }
} // Vectrix