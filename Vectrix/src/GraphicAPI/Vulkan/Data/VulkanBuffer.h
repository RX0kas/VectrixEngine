#pragma once

#include "volk.h"

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

        VulkanBuffer(const VulkanBuffer&) = delete;
        VulkanBuffer& operator=(const VulkanBuffer&) = delete;

        /// Maps the whole buffer, which has to be host visible
        VkResult map();
        void unmap();

        void writeToBuffer(const void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

        [[nodiscard]] VkBuffer getBuffer() const { return m_buffer; }
    private:
        static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

        Device& m_device;
        void* m_mapped = nullptr;
        VkBuffer m_buffer = VK_NULL_HANDLE;
        VmaAllocation m_allocation = VK_NULL_HANDLE;

        VkDeviceSize m_bufferSize;
    };

    /// How the vertices described by the layout are read: one binding, advancing per vertex
    std::vector<VkVertexInputBindingDescription> getVertexBindingDescriptions(const BufferLayout& layout);

    /// One vertex attribute per element of the layout, at consecutive locations
    std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions(const BufferLayout& layout);
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
        return fmt::format_to(ctx.out(), "{}", ss.str());
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