#ifndef VECTRIXWORKSPACE_VULKANVERTEXARRAY_H
#define VECTRIXWORKSPACE_VULKANVERTEXARRAY_H
#include "Vectrix/Core/Core.h"
#include "Vectrix/Rendering/Mesh/MeshHandle.h"
#include "Vectrix/Rendering/Mesh/VertexArray.h"

namespace Vectrix {
    class VulkanVertexArray : public VertexArray {
    public:
        [[nodiscard]] MeshHandle getHandle() const { return m_handle; }
    protected:
        void setHandle(MeshHandle handle) override { m_handle = handle; }
    private:
        MeshHandle m_handle{};
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_VULKANVERTEXARRAY_H