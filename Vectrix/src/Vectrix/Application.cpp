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
#include "Utils/Folders.h"
#include "Vectrix/Utils/Path.h"


namespace Vectrix {
	namespace {
		// From the bottom up: the layers in the order they were pushed, then the overlays, so what is pushed later
		// draws on top. By index, re-reading the size, and holding each layer during its call: a callback that
		// pushes or pops a layer can't invalidate the loop (a layer pushed now is called this frame too)
		template<typename Fn>
		void forEachLayer(const LayerStack& stack, Fn&& fn) {
			for (const std::vector<std::shared_ptr<Layer>>* group : {&stack.layers(), &stack.overlays()}) {
				for (const auto& layer : *group) {
						fn(layer);
				}
			}
		}

		// From the top down, for events: the last overlay pushed first, the first layer pushed last. Stops once
		// the event is handled
		void dispatchTopDown(const LayerStack& stack, Event& e) {
			for (const std::vector<std::shared_ptr<Layer>>* group : {&stack.overlays(), &stack.layers()}) {
				for (size_t i = group->size(); i-- > 0 && !e.Handled;) {
					if (i >= group->size())
						continue; // a layer removed by an earlier call
					const std::shared_ptr<Layer> layer = (*group)[i];
					layer->OnEvent(e);
				}
			}
		}
	}

	Application* Application::s_instance = nullptr;

	Application::Application() {
		VC_PROFILER_FUNCTION();
		VC_CORE_ASSERT(!s_instance, "Application already exists!");
		s_instance = this;

		// Created and loaded before the window: the graphics backend reads settings
		// during init(). A per-project load layers on top of this later (openScene).
		m_settingsManager = std::shared_ptr<SettingsManager>(new SettingsManager());
		{
			const std::filesystem::path configFolder = getConfigFolder();
			std::error_code ec;
			std::filesystem::create_directories(configFolder, ec); // loadGlobal tolerates a missing file, but save() needs the folder to exist

			const auto globalPath = configFolder / settingsFileName;
			if (const auto [result, message] = m_settingsManager->loadGlobal(globalPath); result != SUCCESS)
				VC_CORE_WARN("Global settings not loaded ({}): {}", toUtf8(globalPath), message);
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
		m_window->setEventCallback(VC_BIND_EVENT_FN(onEvent));
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
		// The last frame may still be executing: nothing it uses (layers' framebuffers, textures, ImGui
		// descriptors) can be destroyed before the GPU is done with it
		GraphicsContext::waitIdle();
		forEachLayer(m_layerStack, [](const std::shared_ptr<Layer>& layer) { layer->OnDetach(); });
		m_layerStack.destroy();
		m_assetsManager.reset();
		m_imGuiLayer.reset();
		m_window.reset();
	}

	void Application::onEvent(Event& e) {
		VC_PROFILER_FUNCTION();
		// The ImGuiLayer is owned apart from the stack but sits on top of everything: it gets the
		// first look so it can swallow the mouse/keyboard events ImGui wants (see startBlockEvents)
		if (m_imGuiLayer)
			m_imGuiLayer->OnEvent(e);

		dispatchTopDown(m_layerStack, e);

		// Layers see a close request first: one that handles it (e.g. to ask about unsaved work) keeps the
		// application running
		if (!e.Handled) {
			EventDispatcher dispatcher(e);
			dispatcher.Dispatch<WindowCloseEvent>(VC_BIND_EVENT_FN_RETURN(onWindowClose));
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

			if (m_hasToSwitch) {
				// The previous frame can still be drawing with the old layer's resources, which go away with it
				GraphicsContext::waitIdle();
				// The new layer takes the old one's place, so the order of the other layers doesn't change
				if (const std::shared_ptr<Layer> oldLayer = m_layerStack.ReplaceLayer(m_oldLayer, m_nextLayer))
					oldLayer->OnDetach();
				if (m_dataToNextLayer.empty()) m_nextLayer->OnAttach();
				else m_nextLayer->OnAttach(m_dataToNextLayer);

				m_oldLayer = nullptr;
				m_nextLayer.reset();
				m_dataToNextLayer = JsonObject();
				m_hasToSwitch = false;
			}

			forEachLayer(m_layerStack, [this](const std::shared_ptr<Layer>& layer) { layer->OnUpdate(m_deltaTime); });
			if (RenderCommand::canRender()) {
				forEachLayer(m_layerStack, [](const std::shared_ptr<Layer>& layer) { layer->OnRenderOffscreen(); });

				RenderCommand::beginFrame();
				forEachLayer(m_layerStack, [](const std::shared_ptr<Layer>& layer) { layer->OnRender(); });
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
		forEachLayer(m_layerStack, [](const std::shared_ptr<Layer>& layer) { layer->OnImGuiRender(); });
		m_imGuiLayer->OnImGuiRender();
	}
}
