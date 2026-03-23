#ifndef VECTRIXWORKSPACE_VULKANVERTEXARRAY_H
#define VECTRIXWORKSPACE_VULKANVERTEXARRAY_H
#include "Vectrix/Core/Core.h"
#include "Vectrix/Rendering/Mesh/Model.h"
#include "Vectrix/Rendering/Mesh/VertexArray.h"

namespace Vectrix {
    class VulkanVertexArray : public VertexArray {
    public:
        VulkanVertexArray();
        ~VulkanVertexArray() override;

        void bind() const override;

        void addVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
        void setIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

        [[nodiscard]] std::vector<Ref<VertexBuffer>> getVertexBuffers() const override { return m_vertexBuffers; }
        [[nodiscard]] const Ref<IndexBuffer>& getIndexBuffer() const override { return m_indexBuffer; }
        [[nodiscard]] MeshHandle getHandle() const { return m_handle; }
        void setHandle(MeshHandle handle) {
            if (hasHandle) {
                VC_CORE_WARN("Changing the handle of a vertex array that already have it");
            }
            m_handle = handle;
            hasHandle = true;
        }
    private:
        bool hasHandle = false;
        MeshHandle m_handle{};
        std::vector<Ref<VertexBuffer>> m_vertexBuffers;
        Ref<IndexBuffer> m_indexBuffer;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_VULKANVERTEXARRAY_H