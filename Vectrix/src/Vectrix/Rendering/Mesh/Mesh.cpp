#include "Mesh.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "GraphicAPI/Vulkan/Rendering/Mesh/MeshRegistry.h"
#include "GraphicAPI/Vulkan/Rendering/Mesh/VulkanVertexArray.h"
#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Rendering/Renderer.h"

namespace Vectrix {
    Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t>& indices) : m_aabb(vertices) {
        VC_PROFILER_FUNCTION();
        m_vertexArray = VertexArray::create();

        // The geometry is only uploaded into the renderer's shared buffers, which is what gets drawn from
        switch (Renderer::getAPI()) {
            case RendererAPI::API::Vulkan:
                m_vertexArray->setHandle(VulkanContext::instance().getMeshRegistry().uploadMesh(vertices, indices));
                break;
            case RendererAPI::API::None:
                VC_CORE_ERROR("Can't register Mesh because RendererAPI is None");
                break;
        }
    }

    Mesh::~Mesh() {
        // Without it the space stayed taken for the whole session, e.g. by every mesh of the projects closed since
        if (Renderer::getAPI() == RendererAPI::API::Vulkan && VulkanContext::exists())
            VulkanContext::instance().getMeshRegistry().releaseMesh(std::static_pointer_cast<VulkanVertexArray>(m_vertexArray)->getHandle());
    }
} // Vectrix
