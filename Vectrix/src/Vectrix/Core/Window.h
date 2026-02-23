#pragma once

#include <utility>

#include "vcpch.h"

#include "Core.h"
#include "Vectrix/Events/Event.h"
#include "Vectrix/Rendering/GraphicsContext.h"

namespace Vectrix {
	struct WindowAttributes
	{
		std::string title;
		unsigned int width;
		unsigned int height;

		WindowAttributes(std::string  title = "Vectrix Engine",
			unsigned int width = 1280,
			unsigned int height = 720)
			: title(std::move(title)), width(width), height(height)
		{
		}
	};

	// Interface representing a desktop system based Window
	class Vectrix_API Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual void init(const WindowAttributes& data = WindowAttributes()) = 0;
		virtual ~Window() = default;

		virtual void onUpdate() = 0;

		[[nodiscard]] virtual unsigned int getWidth() const = 0;
		[[nodiscard]] virtual unsigned int getHeight() const = 0;
		[[nodiscard]] virtual float getAspect() const = 0;

		// Window attributes
		virtual void setEventCallback(const EventCallbackFn& callback) = 0;
		virtual void setVSync(bool enabled) = 0;
		[[nodiscard]] virtual bool isVSync() const = 0;

		[[nodiscard]] virtual void* getNativeWindow() const = 0;
		[[nodiscard]] virtual bool wasWindowResized() const = 0;
		virtual void resetWindowResizedFlag() = 0;

		virtual void show() = 0;
		virtual void hide() = 0;
		[[nodiscard]] virtual bool isVisible() const = 0;
	protected:
		static GraphicsContext* createGraphicContext(GLFWwindow* window) {return GraphicsContext::create(window);}
		friend class Application;
		static Window* create();
		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;
			bool windowResized;
			EventCallbackFn EventCallback;
			bool visible;
		};
	};
}
