#include "vcpch.h"
#include "DebugUtils.h"

#include <algorithm>
#include <cstring>

#include "Vectrix/Core/Log.h"

namespace Vectrix::DebugUtils {
	namespace {
		bool s_enabled = false;
		std::vector<Tool> s_tools;

		std::string purposesToString(const VkToolPurposeFlags purposes) {
			static constexpr std::pair<VkToolPurposeFlagBits, const char*> k_names[] = {
				{VK_TOOL_PURPOSE_VALIDATION_BIT, "validation"},
				{VK_TOOL_PURPOSE_PROFILING_BIT, "profiling"},
				{VK_TOOL_PURPOSE_TRACING_BIT, "tracing"},
				{VK_TOOL_PURPOSE_ADDITIONAL_FEATURES_BIT, "additional features"},
				{VK_TOOL_PURPOSE_MODIFYING_FEATURES_BIT, "modifying features"},
				{VK_TOOL_PURPOSE_DEBUG_REPORTING_BIT_EXT, "debug reporting"},
				{VK_TOOL_PURPOSE_DEBUG_MARKERS_BIT_EXT, "debug markers"},
			};
			std::string text;
			for (const auto& [bit, name] : k_names) {
				if (!(purposes & bit))
					continue;
				if (!text.empty())
					text += ", ";
				text += name;
			}
			return text;
		}
	}

	void init(const bool enabled) {
		// The function pointers come from volkLoadInstance: null when the extension isn't enabled
		s_enabled = enabled && vkSetDebugUtilsObjectNameEXT != nullptr && vkCmdBeginDebugUtilsLabelEXT != nullptr;
	}

	bool isEnabled() {
		return s_enabled;
	}

	void setName(VkDevice device, const VkObjectType type, const uint64_t handle, const std::string_view name) {
		if (!s_enabled || handle == 0)
			return;
		const std::string text(name); // Null-terminated
		VkDebugUtilsObjectNameInfoEXT info{VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
		info.objectType = type;
		info.objectHandle = handle;
		info.pObjectName = text.c_str();
		vkSetDebugUtilsObjectNameEXT(device, &info);
	}

	void beginLabel(VkCommandBuffer cmd, const std::string_view name, const float r, const float g, const float b) {
		if (!s_enabled)
			return;
		// Called for every pass and batch of every frame: a stack buffer rather than a std::string
		char text[256];
		const size_t length = std::min(name.size(), sizeof(text) - 1);
		std::memcpy(text, name.data(), length);
		text[length] = '\0';
		VkDebugUtilsLabelEXT label{VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT};
		label.pLabelName = text;
		label.color[0] = r;
		label.color[1] = g;
		label.color[2] = b;
		label.color[3] = 1.0f;
		vkCmdBeginDebugUtilsLabelEXT(cmd, &label);
	}

	void endLabel(VkCommandBuffer cmd) {
		if (!s_enabled)
			return;
		vkCmdEndDebugUtilsLabelEXT(cmd);
	}

	void detectTools(VkPhysicalDevice physicalDevice) {
		s_tools.clear();
		if (vkGetPhysicalDeviceToolProperties == nullptr)
			return; // Vulkan 1.3 core, so only a loader older than the API the engine asks for lacks it
		uint32_t count = 0;
		if (vkGetPhysicalDeviceToolProperties(physicalDevice, &count, nullptr) != VK_SUCCESS || count == 0)
			return;
		std::vector<VkPhysicalDeviceToolProperties> properties(count, {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TOOL_PROPERTIES});
		if (vkGetPhysicalDeviceToolProperties(physicalDevice, &count, properties.data()) != VK_SUCCESS)
			return;
		properties.resize(count);
		for (const VkPhysicalDeviceToolProperties& p : properties) {
			Tool tool{p.name, p.version, purposesToString(p.purposes)};
			VC_CORE_INFO("Attached tool: {} {} ({})", tool.name, tool.version, tool.purposes);
			s_tools.push_back(std::move(tool));
		}
	}

	const std::vector<Tool>& getTools() {
		return s_tools;
	}
}
