#ifndef VECTRIXWORKSPACE_VERTEXARRAY_H
#define VECTRIXWORKSPACE_VERTEXARRAY_H

#include <memory>

#include "GraphicAPI/Vulkan/rendering/VulkanBuffer.h"
#include "Vectrix/Renderer/Buffer.h"

namespace Vectrix {
    // The combination of an index (optional) and vertex buffer
    class VertexArray {
    public:
        virtual ~VertexArray() {}

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        virtual void addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
        virtual void setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;

        virtual std::vector<std::shared_ptr<VertexBuffer>> getVertexBuffers() const = 0;
        virtual const std::shared_ptr<IndexBuffer>& getIndexBuffer() const = 0;

        static VertexArray* create();
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_VERTEXARRAY_H