#ifndef VECTRIXWORKSPACE_VULKANVERTEXARRAY_H
#define VECTRIXWORKSPACE_VULKANVERTEXARRAY_H
#include "Vectrix/Renderer/VertexArray.h"

namespace Vectrix {
    class VulkanVertexArray : public VertexArray {
    public:
        VulkanVertexArray();
        ~VulkanVertexArray() override;

        void bind() const override;
        void unbind() const override;

        void addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
        void setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

        std::vector<std::shared_ptr<VertexBuffer>> getVertexBuffers() const override { return m_VertexBuffers; }
        const std::shared_ptr<IndexBuffer>& getIndexBuffer() const override { return m_IndexBuffer; }
    private:
        std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
        std::shared_ptr<IndexBuffer> m_IndexBuffer;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_VULKANVERTEXARRAY_H