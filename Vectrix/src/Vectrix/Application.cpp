#include "vcpch.h"

#include "Application.h"

#include <filesystem>
#include <memory>

#include "Core/DeltaTime.h"
#include "Debug/Profiler.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Rendering/GraphicsContext.h"
#include "Rendering/RenderCommand.h"
#include "Rendering/Renderer.h"
#include "Rendering/Textures/TextureManager.h"
#include "Settings/SettingsManager.h"


namespace Vectrix {

#define BIND_EVENT_FN(x) [this](auto && PH1) { x(std::forward<decltype(PH1)>(PH1)); }

	Application* Application::s_instance = nullptr;

	Application::Application() {
		VC_PROFILER_FUNCTION();
		VC_CORE_ASSERT(!s_instance, "Application already exists!");
		s_instance = this;

		// Created and loaded before the window: the graphics backend reads settings
		// during init(). A per-project load layers on top of this later (openScene).
		m_settingsManager = std::shared_ptr<SettingsManager>(new SettingsManager());
		{
			const auto globalPath = std::filesystem::current_path() / settingsFileName;
			if (const auto [result, message] = m_settingsManager->loadGlobal(globalPath); result != SUCCESS)
				VC_CORE_WARN("Global settings not loaded ({}): {}", globalPath.string(), message);
		}

		// Apply the settings that are consumed before the graphics backend comes up.
		const JsonObject& startupSettings = SettingsManager::getSettings();

		WindowAttributes windowAttributes;
		std::string windowTitle;
		if (const auto windowIt = startupSettings.find("window"); windowIt != startupSettings.end()) {
			const JsonValue& w = windowIt->second;
			windowAttributes.width  = w["width"].getAs<uint32_t>().value_or(windowAttributes.width);
			windowAttributes.height = w["height"].getAs<uint32_t>().value_or(windowAttributes.height);
			windowTitle = w["title"].getAs<std::string>().value_or("");
		}

#ifdef VC_DEBUG
		// Logging is only compiled in on a debug build.
		if (const auto engineIt = startupSettings.find("engine"); engineIt != startupSettings.end()) {
			if (const auto level = engineIt->second["logging"]["level"].getAs<std::string>()) {
				const auto parsed = spdlog::level::from_str(*level);
				if (parsed != spdlog::level::off || *level == "off") {
					Log::getCoreLogger()->set_level(parsed);
					Log::getClientLogger()->set_level(parsed);
				}
			}
		}
#endif

		m_window = std::unique_ptr<Window>(Window::create());
		m_window->setEventCallback(BIND_EVENT_FN(onEvent));
		m_window->init(windowAttributes);
		if (!windowTitle.empty())
			m_window->setTitle(windowTitle);

		m_assetsManager = std::make_unique<AssetsManager>();

		auto i = new ImGuiLayer();
		m_imGuiLayer = std::unique_ptr<ImGuiLayer>(i);
		m_imGuiLayer->OnAttach();

		Renderer::initOutline();
	}

	Application::~Application() {
		VC_PROFILER_FUNCTION();
		m_layerStack.destroy();
		m_assetsManager.reset();
		m_imGuiLayer.reset();
		m_window.reset();
	}

	void Application::onEvent(Event& e) {
		VC_PROFILER_FUNCTION();
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(VC_BIND_EVENT_FN_RETURN(onWindowClose));

		for (auto it = m_layerStack.end(); it != m_layerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::run() {
		VC_PROFILER_FUNCTION();
		m_window->show();
		Renderer::resizeMask({m_window->getWidth(),m_window->getHeight()});
		while (m_running) {
			const auto time = static_cast<float>(glfwGetTime());
			m_deltaTime = time - m_LastFrameTime;
			m_LastFrameTime = time;
			for (auto& layer : m_layerStack) {
				layer->OnUpdate(m_deltaTime);
			}
			if (RenderCommand::canRender()) {
				for (auto& layer : m_layerStack) {
					layer->OnRenderOffscreen();
				}

				RenderCommand::beginFrame();
				for (const std::shared_ptr<Layer>& layer : m_layerStack) {
					layer->OnRender();
				}
				RenderCommand::endFrame();

				m_imGuiLayer->OnRender();
				m_imGuiLayer->OnUpdate(m_deltaTime);

				RenderCommand::sendFrame();
			}

			m_window->onUpdate();
		}
	}

	bool Application::onWindowClose(WindowCloseEvent& e) {
		VC_PROFILER_FUNCTION();
		m_running = false;
		return true;
	}

	void Application::PushLayer(const std::shared_ptr<Layer>& layer) {
		VC_PROFILER_FUNCTION();
		m_layerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(const std::shared_ptr<Layer>& layer) {
		VC_PROFILER_FUNCTION();
		m_layerStack.PushOverlay(layer);
		layer->OnAttach();
	}


	void Application::renderImGui() {
		VC_PROFILER_FUNCTION();
		for (const std::shared_ptr<Layer>& layer : m_layerStack)
			layer->OnImGuiRender();
		m_imGuiLayer->OnImGuiRender();
	}
}
