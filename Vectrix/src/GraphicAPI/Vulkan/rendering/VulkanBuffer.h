#pragma once

#include "vulkan/vulkan.h"
#include "GraphicAPI/Vulkan/rendering/Device.h"
#include "GraphicAPI/Vulkan/rendering/Vertex.h"

#include "Vectrix/Renderer/Buffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/generated/vk_enum_string_helper.h>


//Format of the attributeDescriptions
#define VULKAN_FORMAT_VEC2 VK_FORMAT_R32G32_SFLOAT
#define VULKAN_FORMAT_VEC3 VK_FORMAT_R32G32B32_SFLOAT

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

        void cleanup();

        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void unmap();

        void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        void writeToIndex(void* data, int index);
        VkResult flushIndex(int index);
        VkDescriptorBufferInfo descriptorInfoForIndex(int index);
        VkResult invalidateIndex(int index);

        VkBuffer getBuffer() const { return buffer; }
        void* getMappedMemory() const { return mapped; }
        uint32_t getInstanceCount() const { return instanceCount; }
        VkDeviceSize getInstanceSize() const { return instanceSize; }
        VkDeviceSize getAlignmentSize() const { return instanceSize; }
        VkBufferUsageFlags getUsageFlags() const { return usageFlags; }
        VkMemoryPropertyFlags getMemoryPropertyFlags() const { return memoryPropertyFlags; }
        VkDeviceSize getBufferSize() const { return bufferSize; }

    private:
        static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

        Device& device;
        void* mapped = nullptr;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;

        VkDeviceSize bufferSize;
        uint32_t instanceCount;
        VkDeviceSize instanceSize;
        VkDeviceSize alignmentSize;
        VkBufferUsageFlags usageFlags;
        VkMemoryPropertyFlags memoryPropertyFlags;
    };
    
    class VulkanVertexBuffer : public VertexBuffer {
    public:
        VulkanVertexBuffer(const std::vector<Vertex>& vertices, uint32_t size);
        virtual ~VulkanVertexBuffer();

        void draw();

        void bind() override;
        //virtual void unbind();
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions(const BufferLayout& layout);
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(const BufferLayout& layout);
        [[nodiscard]] const BufferLayout& getLayout() const override { return _Layout; }
        void setLayout(const BufferLayout& layout) override { VC_CORE_TRACE(layout); _Layout = layout; }
    private:
        uint32_t _vertexCount{ 0 };
        std::unique_ptr<Buffer> buffer;
        bool _enable = false;
        BufferLayout _Layout;
    };

    class VulkanIndexBuffer : public IndexBuffer {
    public:
        VulkanIndexBuffer(uint32_t* indices, uint32_t count);
        ~VulkanIndexBuffer() override;

        void draw();

        void bind() override;
        //virtual void unbind();

        [[nodiscard]] uint32_t getCount() const override { return _IndexCount; }
    private:
        std::unique_ptr<Buffer> buffer;
        uint32_t _IndexCount;
        bool _enable = false;
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
struct fmt::formatter<Vectrix::Vertex> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Vectrix::Vertex& e, FormatContext& ctx) const {
        return fmt::format_to(
            ctx.out(),
            "Position: {}; Color: {}",
            e.position,
            e.color
        );
    }
};


template <>
struct fmt::formatter<std::vector<Vectrix::Vertex>> : fmt::formatter<std::string> {
    template <typename FormatContext>
    auto format(const std::vector<Vectrix::Vertex>& e, FormatContext& ctx) const {
        std::stringstream s;
		s << "Vertices {";
        for (const auto& vertex : e) {
            s << "\nPosition: [" << vertex.position.x << ";" << vertex.position.y << "] Color: [" << vertex.color.x << ";" << vertex.color.y << ";" << vertex.color.z << "]";
		}
		s << "\n}";

        return fmt::formatter<std::string>::format(s.str(), ctx);
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
struct fmt::formatter<VkFormat> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const VkFormat& f, FormatContext& ctx) const {
        return fmt::format_to(
            ctx.out(),
            "VkFormat({})",
            string_VkFormat(f)
        );
    }
};