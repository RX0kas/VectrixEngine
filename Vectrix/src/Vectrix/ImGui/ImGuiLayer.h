#pragma once
#include "Vectrix/Layers/Layer.h"
#include "GraphicAPI/Vulkan/ImGui/VulkanImGuiManager.h"

namespace Vectrix {

	class Vectrix_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() override;

		void OnAttach() override;
		void OnDetach() override;
		void OnImGuiRender() override;
		void OnUpdate() override;
	private:
		VulkanImGuiManager manager;
	};

}
