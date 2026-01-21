#ifndef VECTRIXWORKSPACE_VULKANVERTEXARRAY_H
#define VECTRIXWORKSPACE_VULKANVERTEXARRAY_H
#include "../../../Vectrix/Renderer/Models/VertexArray.h"

namespace Vectrix {
    class VulkanVertexArray : public VertexArray {
    public:
        VulkanVertexArray();
        ~VulkanVertexArray() override;

        void bind() const override;
        void unbind() const override;

        void addVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
        void setIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

        std::vector<Ref<VertexBuffer>> getVertexBuffers() const override { return m_VertexBuffers; }
        const Ref<IndexBuffer>& getIndexBuffer() const override { return m_IndexBuffer; }
    private:
        std::vector<Ref<VertexBuffer>> m_VertexBuffers;
        Ref<IndexBuffer> m_IndexBuffer;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_VULKANVERTEXARRAY_H