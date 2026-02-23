#ifndef VECTRIXWORKSPACE_VULKANVERTEXARRAY_H
#define VECTRIXWORKSPACE_VULKANVERTEXARRAY_H
#include "Vectrix/Rendering/Models/VertexArray.h"

namespace Vectrix {
    class VulkanVertexArray : public VertexArray {
    public:
        VulkanVertexArray();
        ~VulkanVertexArray() override;

        void bind() const override;
        void unbind() const override;

        void addVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
        void setIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

        [[nodiscard]] std::vector<Ref<VertexBuffer>> getVertexBuffers() const override { return m_vertexBuffers; }
        [[nodiscard]] const Ref<IndexBuffer>& getIndexBuffer() const override { return m_indexBuffer; }
    private:
        std::vector<Ref<VertexBuffer>> m_vertexBuffers;
        Ref<IndexBuffer> m_indexBuffer;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_VULKANVERTEXARRAY_H