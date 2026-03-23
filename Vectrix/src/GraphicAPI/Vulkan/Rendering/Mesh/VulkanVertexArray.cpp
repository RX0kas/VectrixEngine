#include "GraphicAPI/Vulkan/Rendering/Mesh/VulkanVertexArray.h"

#include "Vectrix/Core/Log.h"
#include "Vectrix/Debug/Profiler.h"

namespace Vectrix {
    VulkanVertexArray::VulkanVertexArray() = default;

    VulkanVertexArray::~VulkanVertexArray() = default;

    void VulkanVertexArray::bind() const {
        VC_PROFILER_FUNCTION();
        for (auto& buffer : m_vertexBuffers)
            buffer->bind();
        if (m_indexBuffer)
            m_indexBuffer->bind();
    }

    void VulkanVertexArray::addVertexBuffer(const Ref<VertexBuffer> &vertexBuffer) {
        VC_PROFILER_FUNCTION();
        VC_CORE_ASSERT(!vertexBuffer->getLayout().getElements().empty(),"Vertex Buffer has no layout!");

        m_vertexBuffers.push_back(vertexBuffer);
    }

    void VulkanVertexArray::setIndexBuffer(const Ref<IndexBuffer> &indexBuffer) {
        VC_PROFILER_FUNCTION();
        m_indexBuffer.reset();
        m_indexBuffer = indexBuffer;
    }
} // Vectrix