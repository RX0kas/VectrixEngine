#include "VulkanVertexArray.h"

#include "../../../Vectrix/Core/Log.h"

namespace Vectrix {
    VulkanVertexArray::VulkanVertexArray() {

    }

    VulkanVertexArray::~VulkanVertexArray() {

    }

    void VulkanVertexArray::bind() const {
        for (auto& buffer : m_VertexBuffers)
            buffer->bind();
        if (m_IndexBuffer)
            m_IndexBuffer->bind();
    }

    void VulkanVertexArray::unbind() const {

    }

    void VulkanVertexArray::addVertexBuffer(const Ref<VertexBuffer> &vertexBuffer) {
        VC_CORE_ASSERT(!vertexBuffer->getLayout().getElements().empty(),"Vertex Buffer has no layout!");

        m_VertexBuffers.push_back(vertexBuffer);
    }

    void VulkanVertexArray::setIndexBuffer(const Ref<IndexBuffer> &indexBuffer) {
        m_IndexBuffer.reset();
        m_IndexBuffer = indexBuffer;
    }
} // Vectrix