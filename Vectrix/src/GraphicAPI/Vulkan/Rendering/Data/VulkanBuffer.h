#pragma once

#include "vulkan/vulkan.h"
#include <sstream>
#include "../Core/Device.h"
#include "Vectrix/Rendering/Mesh/Vertex.h"

#include "Vectrix/Rendering/Buffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "GraphicAPI/Vulkan/Enum_str.h"


namespace Vectrix {
    class VulkanBuffer {
    public:
        VulkanBuffer(
            VkDeviceSize instanceSize,
            uint32_t instanceCount,
            VkBufferUsageFlags usageFlags,
            VkMemoryPropertyFlags memoryPropertyFlags,
            VkDeviceSize minOffsetAlignment = 1);
        ~VulkanBuffer();

        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void unmap();

        void writeToBuffer(const void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
        VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
        VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
        VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

        VkResult flushIndex(int index) const;
        VkDescriptorBufferInfo descriptorInfoForIndex(int index) const;
        VkResult invalidateIndex(int index) const;

        [[nodiscard]] VkBuffer getBuffer() const { return m_buffer; }
        [[nodiscard]] void* getMappedMemory() const { return m_mapped; }
        [[nodiscard]] uint32_t getInstanceCount() const { return m_instanceCount; }
        [[nodiscard]] VkDeviceSize getInstanceSize() const { return m_instanceSize; }
        [[nodiscard]] VkDeviceSize getAlignmentSize() const { return m_instanceSize; }
        [[nodiscard]] VkBufferUsageFlags getUsageFlags() const { return m_usageFlags; }
        [[nodiscard]] VkMemoryPropertyFlags getMemoryPropertyFlags() const { return m_memoryPropertyFlags; }
        [[nodiscard]] VkDeviceSize getBufferSize() const { return m_bufferSize; }
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
        VulkanVertexBuffer(std::vector<Vertex> vertices, uint32_t size);
        ~VulkanVertexBuffer() override;

        void draw() const;

        void bind() override;
        //virtual void unbind();
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions(const BufferLayout& layout);
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(const BufferLayout& layout);
        [[nodiscard]] const BufferLayout& getLayout() const override { return m_layout; }
        void setLayout(const BufferLayout& layout) override { m_layout = layout; }
        [[nodiscard]] uint32_t getVertexCount() const { return m_vertexCount; }
    private:
        uint32_t m_vertexCount{ 0 };
        std::shared_ptr<VulkanBuffer> m_buffer;
        bool m_enable = false;
        BufferLayout m_layout;
    };

    class VulkanIndexBuffer : public IndexBuffer {
    public:
        VulkanIndexBuffer(uint32_t* indices, uint32_t count);
        ~VulkanIndexBuffer() override;

        void draw() const;

        void bind() override;
        //virtual void unbind();

        [[nodiscard]] uint32_t getCount() const override { return m_indexCount; }
    private:
        std::shared_ptr<VulkanBuffer> m_buffer;
        uint32_t m_indexCount;
        bool m_enable = false;
    };

    class VulkanUniformBuffer {
    public:
        VulkanUniformBuffer();

    };
}

template <>
struct fmt::formatter<glm::vec2> {
    static constexpr auto parse(const format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const glm::vec2& v, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "({}, {})", v.x, v.y);
    }
};


template <>
struct fmt::formatter<Vectrix::BufferLayout> {
    static constexpr auto parse(const format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Vectrix::BufferLayout& b, FormatContext& ctx) const {
        std::stringstream ss;
        ss << "BufferLayout (Stride=" << b.getStride() << ")\n";
        for (const auto& e : b.getElements())
        {
            ss << "  - " << e.name << " (" << e.size << " bytes)\n";
        }
        return fmt::format_to(
            ctx.out(),
            ss.str()
        );
    }
};

template <>
struct fmt::formatter<VkResult> : fmt::formatter<std::string> {
    static constexpr auto parse(const format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const VkResult& e, FormatContext& ctx) const {
        return fmt::formatter<std::string>::format(Vectrix::string_VkResult(e), ctx);
    }
};