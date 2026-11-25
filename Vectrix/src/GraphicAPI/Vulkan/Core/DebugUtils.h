#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <cstdint>

#include "volk.h"

namespace Vectrix {
	/**
	 * Names and labels for GPU debuggers (RenderDoc, Nsight Graphics), through VK_EXT_debug_utils.
	 *
	 * The extension is enabled whenever the loader offers it (a capture tool's layer does), not only with the
	 * validation layers: a capture then shows "Texture models/crate.png" or "Pass Viewport" instead of
	 * "Image 42". Without it every function here does nothing, so callers don't check.
	 */
	namespace DebugUtils {
		/// Called by Device once the instance exists: whether VK_EXT_debug_utils was enabled on it
		void init(bool enabled);
		[[nodiscard]] bool isEnabled();

		/// Give a Vulkan object a name, shown by the debugger and in validation messages
		void setName(VkDevice device, VkObjectType type, uint64_t handle, std::string_view name);

		template<typename Handle>
		void setName(VkDevice device, VkObjectType type, Handle handle, std::string_view name) {
			setName(device, type, reinterpret_cast<uint64_t>(handle), name);
		}

		/// Start a labelled region of a command buffer (a pass, a batch...), closed by endLabel
		void beginLabel(VkCommandBuffer cmd, std::string_view name, float r = 0.4f, float g = 0.6f, float b = 1.0f);
		void endLabel(VkCommandBuffer cmd);

		/// A label around the scope it lives in
		class ScopedLabel {
		public:
			ScopedLabel(VkCommandBuffer cmd, std::string_view name, float r = 0.4f, float g = 0.6f, float b = 1.0f) : m_cmd(cmd) {
				beginLabel(cmd, name, r, g, b);
			}
			~ScopedLabel() { endLabel(m_cmd); }
			ScopedLabel(const ScopedLabel&) = delete;
			ScopedLabel& operator=(const ScopedLabel&) = delete;
		private:
			VkCommandBuffer m_cmd;
		};

		/// A tool attached to the application (vkGetPhysicalDeviceToolProperties): a debugger, a profiler...
		struct Tool {
			std::string name;
			std::string version;
			std::string purposes; ///< E.g. "tracing, profiling"
		};

		/// Ask the driver which tools are attached, and log them. Called once by Device
		void detectTools(VkPhysicalDevice physicalDevice);
		/// What detectTools found, empty when no tool is attached
		[[nodiscard]] const std::vector<Tool>& getTools();
	}
}
