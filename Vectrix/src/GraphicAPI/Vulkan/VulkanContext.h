#pragma once

#include "Enum_str.h"
#include "Vectrix/Rendering/GraphicsContext.h"
#include "GraphicAPI/Vulkan/Rendering/Core/Device.h"
#include "Rendering/Core/VulkanRenderer.h"
#include "Rendering/Shaders/VulkanShaderCompiler.h"

#define VC_VK_CHECK(x,...) if (x!=VK_SUCCESS) {VC_CORE_ERROR(__VA_ARGS__);}
#define VC_MAKE_VULKAN_COMPATIBLE_VERSION(version) VK_MAKE_API_VERSION(VC_PLATFORM_ID,std::min(getMajor(version),8U), std::min(getMinor(version),12U), std::min(getPatch(version),12U))

namespace Vectrix {
	class MeshRegistry;

	class VulkanContext : public GraphicsContext {
	public:
		VulkanContext(GLFWwindow* windowHandle);
		~VulkanContext() override;
		void init() override;
		void swapBuffers() override;
		void registerMesh(const std::string& name,Ref<Model> model) override;
		static void uploadMeshData();


		[[nodiscard]] Device& getDevice() const { return *m_device; }
		[[nodiscard]] VulkanRenderer& getRenderer() const { return *m_renderer; }
		[[nodiscard]] MeshRegistry& getMeshRegistry() const { return *m_meshRegistry; }
		[[nodiscard]] VulkanShaderCompiler& getCompiler() const { return *m_compiler;}
		[[nodiscard]] VmaAllocator getAllocator() const { return getDevice().getAllocator();}
		static VulkanContext& instance() { return *s_instance; }
		static void check_vk_result(VkResult err) {
			if (err == VK_SUCCESS)
				return;

			VC_CORE_ERROR("VkResult = {0}\n", string_VkResult(err));
		}
	private:
		GLFWwindow* m_WindowHandle;
		Own<Device> m_device;
		Own<VulkanRenderer> m_renderer;
		Own<VulkanShaderCompiler> m_compiler;
		Own<MeshRegistry> m_meshRegistry;

		float getAspect() override {
			return m_renderer->getAspectRatio();
		}
		
		friend class Shader;
		friend class VulkanVertexBuffer;
		friend class VulkanIndexBuffer;
		friend class Application;
	private:
		static VulkanContext* s_instance;

	};
}
