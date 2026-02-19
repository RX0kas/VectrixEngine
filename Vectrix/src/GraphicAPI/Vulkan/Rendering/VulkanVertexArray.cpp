#include "GraphicAPI/Vulkan/Rendering/VulkanVertexArray.h"

#include "Vectrix/Core/Log.h"

namespace Vectrix {
    VulkanVertexArray::VulkanVertexArray() = default;

    VulkanVertexArray::~VulkanVertexArray() = default;

    void VulkanVertexArray::bind() const {
        for (auto& buffer : m_vertexBuffers)
            buffer->bind();
        if (m_indexBuffer)
            m_indexBuffer->bind();
    }

    void VulkanVertexArray::unbind() const {

    }

    void VulkanVertexArray::addVertexBuffer(const Ref<VertexBuffer> &vertexBuffer) {
        VC_CORE_ASSERT(!vertexBuffer->getLayout().getElements().empty(),"Vertex Buffer has no layout!");

        m_vertexBuffers.push_back(vertexBuffer);
    }

    void VulkanVertexArray::setIndexBuffer(const Ref<IndexBuffer> &indexBuffer) {
        m_indexBuffer.reset();
        m_indexBuffer = indexBuffer;
    }
} // Vectrix