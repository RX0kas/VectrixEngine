#include "VulkanComputeShader.h"

#include <cstring>
#include <format>

#include "VulkanContext.h"
#include "Core/DebugUtils.h"
#include "Vectrix/Debug/Profiler.h"

namespace Vectrix {
	VulkanComputeShader::VulkanComputeShader(const std::string& name, const std::string& source)
		: m_device(VulkanContext::instance().getDevice()), m_name(name) {
		VC_PROFILER_FUNCTION();
		createLayouts();
		createDescriptorSet();
		const std::vector<uint32_t> code = VulkanContext::instance().getCompiler().compileCompute(m_name.c_str(), source.c_str(), VulkanContext::settings().shaders.optimize);
		if (code.empty())
			return; // Doesn't compile (logged): ComputeShader::createFromSource reports it
		m_computePipeline = std::make_unique<ComputePipeline>(code, m_pipelineLayout, m_name);
	}

	VulkanComputeShader::~VulkanComputeShader() {
		VC_PROFILER_FUNCTION();
		// dispatch waited for its work, but a frame may have used the pipeline: everything goes once the frames in
		// flight are done
		std::shared_ptr<ComputePipeline> pipeline(std::move(m_computePipeline));
		VulkanContext::destroyWhenUnused([pipeline, buffers = m_buffers, set = m_descriptorSet, setLayout = m_descriptorSetLayout,
		                                  layout = m_pipelineLayout, device = &m_device]() mutable {
			pipeline.reset();
			for (Buffer& buffer : buffers) {
				if (buffer.buffer != VK_NULL_HANDLE)
					vmaDestroyBuffer(device->getSSBOAllocator(), buffer.buffer, buffer.allocation);
			}
			if (set != VK_NULL_HANDLE)
				device->freeDescriptorSets(1, &set);
			if (layout != VK_NULL_HANDLE)
				vkDestroyPipelineLayout(device->device(), layout, nullptr);
			if (setLayout != VK_NULL_HANDLE)
				vkDestroyDescriptorSetLayout(device->device(), setLayout, nullptr);
		});
	}

	void VulkanComputeShader::createLayouts() {
		// Every binding a storage buffer: a shader uses the ones it declares, the others are never read
		std::array<VkDescriptorSetLayoutBinding, k_maxBuffers> bindings{};
		for (uint32_t i = 0; i < k_maxBuffers; ++i) {
			bindings[i].binding = i;
			bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[i].descriptorCount = 1;
			bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		}
		VkDescriptorSetLayoutCreateInfo setLayoutInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
		setLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		setLayoutInfo.pBindings = bindings.data();
		if (vkCreateDescriptorSetLayout(m_device.device(), &setLayoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
			VC_CORE_ERROR_NO_EXIT("Compute shader {}: failed to create the descriptor set layout", m_name);
			m_descriptorSetLayout = VK_NULL_HANDLE;
			return;
		}

		VkPushConstantRange pushConstants{};
		pushConstants.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		pushConstants.offset = 0;
		pushConstants.size = static_cast<uint32_t>(k_maxPushConstantSize);

		VkPipelineLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
		layoutInfo.setLayoutCount = 1;
		layoutInfo.pSetLayouts = &m_descriptorSetLayout;
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &pushConstants;
		if (vkCreatePipelineLayout(m_device.device(), &layoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
			VC_CORE_ERROR_NO_EXIT("Compute shader {}: failed to create the pipeline layout", m_name);
			m_pipelineLayout = VK_NULL_HANDLE;
		}
	}

	void VulkanComputeShader::createDescriptorSet() {
		if (m_descriptorSetLayout == VK_NULL_HANDLE)
			return;
		VkDescriptorSetAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		allocateInfo.descriptorPool = m_device.descriptorPool();
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.pSetLayouts = &m_descriptorSetLayout;
		if (m_device.allocateDescriptorSets(allocateInfo, &m_descriptorSet) != VK_SUCCESS) {
			// The pool doesn't grow: engine.graphicAPI.vulkan.startingDescriptorPoolConfig.ssboCount/maxSets
			VC_CORE_ERROR_NO_EXIT("Compute shader {}: no descriptor set left in the pool (raise ssboCount/maxSets)", m_name);
			m_descriptorSet = VK_NULL_HANDLE;
		}
	}

	bool VulkanComputeShader::ensureBuffer(const uint32_t binding, const size_t size) {
		if (binding >= k_maxBuffers) {
			VC_CORE_ERROR_NO_EXIT("Compute shader {}: binding {} out of range (max {})", m_name, binding, k_maxBuffers - 1);
			return false;
		}
		if (size == 0) {
			VC_CORE_ERROR_NO_EXIT("Compute shader {}: a buffer can't be empty (binding {})", m_name, binding);
			return false;
		}
		if (m_descriptorSet == VK_NULL_HANDLE)
			return false; // Already reported at creation
		Buffer& buffer = m_buffers[binding];
		if (buffer.buffer != VK_NULL_HANDLE && buffer.size == size)
			return true;

		// Every dispatch waited for the GPU: the previous buffer is no longer in use
		destroyBuffer(buffer);

		VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		// Host visible and mapped, for random access: written before a dispatch and read back after it.
		// Simple, not the fastest for data that stays on the GPU between dispatches
		VmaAllocationCreateInfo allocationInfo{};
		allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		VmaAllocationInfo allocated{};
		if (vmaCreateBuffer(m_device.getSSBOAllocator(), &bufferInfo, &allocationInfo, &buffer.buffer, &buffer.allocation, &allocated) != VK_SUCCESS) {
			VC_CORE_ERROR_NO_EXIT("Compute shader {}: failed to create a {} bytes buffer (binding {})", m_name, size, binding);
			buffer = {};
			return false;
		}
		buffer.mapped = allocated.pMappedData;
		buffer.size = size;
		DebugUtils::setName(m_device.device(), VK_OBJECT_TYPE_BUFFER, buffer.buffer, std::format("{} binding {}", m_name, binding));

		VkDescriptorBufferInfo descriptorBuffer{buffer.buffer, 0, VK_WHOLE_SIZE};
		VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
		write.dstSet = m_descriptorSet;
		write.dstBinding = binding;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		write.pBufferInfo = &descriptorBuffer;
		vkUpdateDescriptorSets(m_device.device(), 1, &write, 0, nullptr);
		return true;
	}

	void VulkanComputeShader::destroyBuffer(Buffer& buffer) const {
		if (buffer.buffer != VK_NULL_HANDLE)
			vmaDestroyBuffer(m_device.getSSBOAllocator(), buffer.buffer, buffer.allocation);
		buffer = {};
	}

	bool VulkanComputeShader::setBuffer(const uint32_t binding, const void* data, const size_t size) {
		VC_PROFILER_FUNCTION();
		if (!data || !ensureBuffer(binding, size))
			return false;
		const Buffer& buffer = m_buffers[binding];
		std::memcpy(buffer.mapped, data, size);
		// No-op on coherent memory, needed on the rest
		vmaFlushAllocation(m_device.getSSBOAllocator(), buffer.allocation, 0, VK_WHOLE_SIZE);
		return true;
	}

	bool VulkanComputeShader::allocateBuffer(const uint32_t binding, const size_t size) {
		return ensureBuffer(binding, size);
	}

	size_t VulkanComputeShader::getBufferSize(const uint32_t binding) const {
		return binding < k_maxBuffers ? m_buffers[binding].size : 0;
	}

	bool VulkanComputeShader::setPushConstants(const void* data, const size_t size) {
		if (size > k_maxPushConstantSize || (!data && size > 0)) {
			VC_CORE_ERROR_NO_EXIT("Compute shader {}: {} bytes of push constants, at most {}", m_name, size, k_maxPushConstantSize);
			return false;
		}
		if (size > 0)
			std::memcpy(m_pushConstants.data(), data, size);
		m_pushConstantSize = size;
		return true;
	}

	bool VulkanComputeShader::dispatch(const uint32_t groupsX, const uint32_t groupsY, const uint32_t groupsZ) {
		VC_PROFILER_FUNCTION();
		if (!isValid()) {
			VC_CORE_ERROR_NO_EXIT("Compute shader {} can't run: it didn't build", m_name);
			return false;
		}
		if (groupsX == 0 || groupsY == 0 || groupsZ == 0)
			return true; // Nothing to run

		VkCommandBuffer cmd = m_device.beginSingleTimeCommands();
		{
			DebugUtils::ScopedLabel label(cmd, m_name, 0.9f, 0.5f, 0.2f);
			m_computePipeline->bind(cmd);
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
			if (m_pushConstantSize > 0)
				vkCmdPushConstants(cmd, m_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, static_cast<uint32_t>(m_pushConstantSize), m_pushConstants.data());
			vkCmdDispatch(cmd, groupsX, groupsY, groupsZ);

			// What the shader wrote is made visible to the CPU's reads (readBuffer)
			VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
			barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
		}
		// Submits and waits for this command buffer only (a fence of its own): the frames keep going
		m_device.endSingleTimeCommands(cmd);
		return true;
	}

	bool VulkanComputeShader::readBuffer(const uint32_t binding, void* out, const size_t size, const size_t offset) const {
		VC_PROFILER_FUNCTION();
		if (binding >= k_maxBuffers || m_buffers[binding].buffer == VK_NULL_HANDLE) {
			VC_CORE_ERROR_NO_EXIT("Compute shader {}: no buffer at binding {}", m_name, binding);
			return false;
		}
		const Buffer& buffer = m_buffers[binding];
		if (!out || offset > buffer.size || size > buffer.size - offset) {
			VC_CORE_ERROR_NO_EXIT("Compute shader {}: reading {} bytes at {} from a {} bytes buffer (binding {})", m_name, size, offset, buffer.size, binding);
			return false;
		}
		// No-op on coherent memory, needed on the rest
		vmaInvalidateAllocation(m_device.getSSBOAllocator(), buffer.allocation, offset, size);
		std::memcpy(out, static_cast<const std::byte*>(buffer.mapped) + offset, size);
		return true;
	}
} // Vectrix
