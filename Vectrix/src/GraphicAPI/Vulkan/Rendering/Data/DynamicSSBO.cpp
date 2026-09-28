#include "DynamicSSBO.h"

#include "ShaderSSBO.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
    VkDescriptorSetLayout DynamicSSBO::s_descriptorSetLayout = nullptr;

    DynamicSSBO::DynamicSSBO(const std::shared_ptr<ShaderUniformLayout> &layout, uint32_t initialCapacity) : m_device(VulkanContext::instance().getDevice()), m_capacity(initialCapacity), m_allocator(VulkanContext::instance().getSSBOAllocator()) {
        m_layout = layout;
        m_framesInFlight = SwapChain::MAX_FRAMES_IN_FLIGHT;

        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(m_device.physicalDevice(), &props);
        m_minOffsetAlignment = props.limits.minStorageBufferOffsetAlignment;

        // The elements are an array the shader indexes with the std430 stride, which is the layout's struct size.
        // minStorageBufferOffsetAlignment only constrains descriptor offsets, i.e. where each frame's region starts
        // (see frameSize): padding every element to it would make the CPU and the shader disagree on where object i is
        m_elementStride = m_layout->getStructSize();

        createDescriptorSetLayout();

        allocateGPUBuffer();

        m_storage.resize(frameSize() * m_framesInFlight, 0);

        m_descriptorSets.resize(m_framesInFlight);
        std::vector<VkDescriptorSetLayout> layouts(m_framesInFlight, getStaticDescriptorSetLayout());
        VkDescriptorSetAllocateInfo alloc{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        alloc.descriptorPool = m_device.descriptorPool();
        alloc.descriptorSetCount = m_framesInFlight;
        alloc.pSetLayouts = layouts.data();
        vkAllocateDescriptorSets(m_device.device(), &alloc, m_descriptorSets.data());

        updateDescriptorSets();
    }

    DynamicSSBO::DynamicSSBO(DynamicSSBO&& other) noexcept
    : m_device(other.m_device), m_elementStride(other.m_elementStride), m_minOffsetAlignment(other.m_minOffsetAlignment), m_capacity(other.m_capacity),
        m_framesInFlight(other.m_framesInFlight), m_buffer(other.m_buffer), m_allocation(other.m_allocation), m_allocator(other.m_allocator),
        m_mapped(other.m_mapped), m_storage(std::move(other.m_storage)), m_descriptorSets(std::move(other.m_descriptorSets)) {
        m_layout = other.m_layout;
        other.m_layout = nullptr;
        other.m_buffer = VK_NULL_HANDLE;
        other.m_allocation = VK_NULL_HANDLE;
        other.m_mapped = nullptr;
        other.m_elementStride = 0;
        other.m_capacity = 0;
        other.m_framesInFlight = 0;
    }

    DynamicSSBO::~DynamicSSBO() {
        freeDescriptorSets();
        if (m_allocation == VK_NULL_HANDLE) return;

        if (m_mapped) {
            vmaUnmapMemory(m_allocator, m_allocation);
            m_mapped = nullptr;
        }

        if (m_buffer != VK_NULL_HANDLE) {
            m_device.destroyBuffer(m_buffer, m_allocation, m_allocator);
            m_buffer = VK_NULL_HANDLE;
            m_allocation = VK_NULL_HANDLE;
        }
    }

    DynamicSSBO& DynamicSSBO::operator=(DynamicSSBO&& other) noexcept {
        if (this == &other) return *this;

        freeDescriptorSets();
        if (m_allocation != VK_NULL_HANDLE) {
            if (m_mapped) vmaUnmapMemory(m_allocator, m_allocation);
            m_device.destroyBuffer(m_buffer, m_allocation,m_allocator);
        }

        m_layout = other.m_layout;
        m_elementStride = other.m_elementStride;
        m_minOffsetAlignment = other.m_minOffsetAlignment;
        m_capacity = other.m_capacity;
        m_framesInFlight = other.m_framesInFlight;
        m_buffer = other.m_buffer;
        m_allocation = other.m_allocation;
        m_allocator = other.m_allocator;
        m_mapped = other.m_mapped;
        m_storage = std::move(other.m_storage);
        m_descriptorSets = std::move(other.m_descriptorSets);

        other.m_layout = nullptr;
        other.m_buffer = VK_NULL_HANDLE;
        other.m_allocation = VK_NULL_HANDLE;
        other.m_mapped = nullptr;
        other.m_elementStride = 0;
        other.m_capacity = 0;
        other.m_framesInFlight = 0;

        return *this;
    }

    void DynamicSSBO::write(uint32_t frameIndex, uint32_t elementIndex, const void* src) {
        while (elementIndex >= m_capacity) {
            grow();
        }

        const size_t offset = frameIndex * frameSize() + static_cast<size_t>(elementIndex) * m_elementStride;

        std::memcpy(m_storage.data() + offset, src, m_layout->getStructSize());
    }

    void DynamicSSBO::flush(uint32_t frameIndex, uint32_t elementCount) const {
        VC_CORE_ASSERT(elementCount <= m_capacity, "Flushing {} elements but the DynamicSSBO only holds {}", elementCount, m_capacity);
        // Only the elements written this frame: the GPU never reads past elementCount, so copying the whole
        // frame region (capacity * stride, e.g. ~800 KB for the renderer's batches) every frame is wasted work
        const size_t offset = frameIndex * frameSize();
        const size_t size = static_cast<size_t>(m_elementStride) * elementCount;

        uint8_t* dst = static_cast<uint8_t*>(m_mapped) + offset;
        std::memcpy(dst, m_storage.data() + offset, size);
    }

    void DynamicSSBO::freeDescriptorSets() {
        if (m_descriptorSets.empty()) return;
        // Frames still in flight bind these sets: they can only be freed once the GPU is done
        vkDeviceWaitIdle(m_device.device());
        // The device pool is created with VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
        vkFreeDescriptorSets(m_device.device(), m_device.descriptorPool(), static_cast<uint32_t>(m_descriptorSets.size()), m_descriptorSets.data());
        m_descriptorSets.clear();
    }

    void DynamicSSBO::grow() {
        vkDeviceWaitIdle(m_device.device());
        const size_t oldFrameSize = frameSize();
        m_capacity *= 4;

        if (m_mapped) vmaUnmapMemory(VulkanContext::instance().getSSBOAllocator(), m_allocation);
        m_device.destroyBuffer(m_buffer, m_allocation,m_allocator);
        m_mapped = nullptr;

        // Each frame's region starts at frameIndex * frameSize(): move the elements already written to
        // their new offsets rather than just resizing, which would leave them under the wrong frame
        const size_t newFrameSize = frameSize();
        std::vector<uint8_t> storage(newFrameSize * m_framesInFlight, 0);
        for (uint32_t frame = 0; frame < m_framesInFlight; ++frame)
            std::memcpy(storage.data() + frame * newFrameSize, m_storage.data() + frame * oldFrameSize, oldFrameSize);
        m_storage = std::move(storage);

        allocateGPUBuffer();

        updateDescriptorSets();
    }

    void DynamicSSBO::allocateGPUBuffer() {
        VC_CORE_ASSERT(m_layout != nullptr, "Layout is null in DynamicSSBO::allocateGPUBuffer!");
        VkDeviceSize bufferSize = static_cast<VkDeviceSize>(frameSize()) * m_framesInFlight;
        VC_CORE_ASSERT(bufferSize > 0, "Buffer size is 0 in DynamicSSBO::allocateGPUBuffer!");
        m_device.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_buffer,
            m_allocation,
            m_allocator
        );

        vmaMapMemory(VulkanContext::instance().getSSBOAllocator(), m_allocation, &m_mapped);
        memset(m_mapped, 0, bufferSize);
    }

    void DynamicSSBO::createDescriptorSetLayout() {
        if (s_descriptorSetLayout!=nullptr) return;

        VkDescriptorSetLayoutBinding b{};
        b.binding = 0;
        b.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        b.descriptorCount = 1;
        b.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo li{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        li.bindingCount = 1;
        li.pBindings = &b;
        vkCreateDescriptorSetLayout(VulkanContext::instance().getDevice().device(), &li, nullptr, &s_descriptorSetLayout);
    }

    void DynamicSSBO::updateDescriptorSets() const {
        for (uint32_t frame = 0; frame < m_framesInFlight; ++frame) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_buffer;
            bufferInfo.offset = static_cast<VkDeviceSize>(frame) * frameSize();
            bufferInfo.range = static_cast<VkDeviceSize>(m_elementStride) * m_capacity;

            VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            write.dstSet = m_descriptorSets[frame];
            write.dstBinding = 0;
            write.descriptorCount = 1;
            write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            write.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(m_device.device(), 1, &write, 0, nullptr);
        }
    }
} // Vectrix
