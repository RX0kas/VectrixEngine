#pragma once

#include "vulkan/vulkan.h"
#include "GraphicAPI/Vulkan/Rendering/Device.h"
#include "../../../Vectrix/Rendering/Models/Vertex.h"

#include "Vectrix/Rendering/Buffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "GraphicAPI/Vulkan/Enum_str.h"


namespace Vectrix {
    class Buffer {
    public:
        Buffer(
            VkDeviceSize instanceSize,
            uint32_t instanceCount,
            VkBufferUsageFlags usageFlags,
            VkMemoryPropertyFlags memoryPropertyFlags,
            VkDeviceSize minOffsetAlignment = 1);
        ~Buffer();

        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void unmap();

        void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        VkResult flushIndex(int index);
        VkDescriptorBufferInfo descriptorInfoForIndex(int index);
        VkResult invalidateIndex(int index);

        VkBuffer getBuffer() const { return m_buffer; }
        void* getMappedMemory() const { return m_mapped; }
        uint32_t getInstanceCount() const { return m_instanceCount; }
        VkDeviceSize getInstanceSize() const { return m_instanceSize; }
        VkDeviceSize getAlignmentSize() const { return m_instanceSize; }
        VkBufferUsageFlags getUsageFlags() const { return m_usageFlags; }
        VkMemoryPropertyFlags getMemoryPropertyFlags() const { return m_memoryPropertyFlags; }
        VkDeviceSize getBufferSize() const { return m_bufferSize; }
    private:
        static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

        Device& m_device;
        void* m_mapped = nullptr;
        VkBuffer m_buffer = VK_NULL_HANDLE;
        VmaAllocation m_allocation = VK_NULL_HANDLE;

        VkDeviceSize m_bufferSize;
        uint32_t m_instanceCount;
        VkDeviceSize m_instanceSize;
        VkDeviceSize m_alignmentSize;
        VkBufferUsageFlags m_usageFlags;
        VkMemoryPropertyFlags m_memoryPropertyFlags;
    };
    
    class VulkanVertexBuffer : public VertexBuffer {
    public:
        VulkanVertexBuffer(const std::vector<Vertex>& vertices, uint32_t size);
        ~VulkanVertexBuffer() override;

        void draw();

        void bind() override;
        //virtual void unbind();
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions(const BufferLayout& layout);
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(const BufferLayout& layout);
        [[nodiscard]] const BufferLayout& getLayout() const override { return m_Layout; }
        void setLayout(const BufferLayout& layout) override { m_Layout = layout; }
    private:
        uint32_t _vertexCount{ 0 };
        Ref<Buffer> buffer;
        bool _enable = false;
        BufferLayout m_Layout;
    };

    class VulkanIndexBuffer : public IndexBuffer {
    public:
        VulkanIndexBuffer(const uint32_t* indices, uint32_t count);
        ~VulkanIndexBuffer() override;

        void draw();

        void bind() override;
        //virtual void unbind();

        [[nodiscard]] uint32_t getCount() const override { return _IndexCount; }
    private:
        Ref<Buffer> buffer;
        uint32_t _IndexCount;
        bool _enable = false;
    };

    class VulkanUniformBuffer {
    public:
        VulkanUniformBuffer();

    };
}

template <>
struct fmt::formatter<glm::vec2> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const glm::vec2& v, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "({}, {})", v.x, v.y);
    }
};


template <>
struct fmt::formatter<Vectrix::BufferLayout> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Vectrix::BufferLayout& b, FormatContext& ctx) const {
        std::stringstream ss;
        ss << "BufferLayout (Stride=" << b.getStride() << ")\n";
        for (const auto& e : b.getElements())
        {
            ss << "  - " << e.Name << " (" << e.Size << " bytes)\n";
        }
        return fmt::format_to(
            ctx.out(),
            ss.str()
        );
    }
};

template <>
struct fmt::formatter<VkResult> : fmt::formatter<std::string> {

    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const VkResult& e, FormatContext& ctx) const {
        return fmt::formatter<std::string>::format(Vectrix::string_VkResult(e), ctx);
    }
};