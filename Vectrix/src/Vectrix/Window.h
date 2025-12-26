#pragma once

#include "vcpch.h"

#include "Vectrix/Core.h"
#include "Vectrix/Events/Event.h"

namespace Vectrix {
	struct WindowAttributes
	{
		std::string title;
		unsigned int width;
		unsigned int height;

		WindowAttributes(const std::string& title = "Vectrix Engine",
			unsigned int width = 1280,
			unsigned int height = 720)
			: title(title), width(width), height(height)
		{
		}
	};

	// Interface representing a desktop system based Window
	class Vectrix_API Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual void init(const WindowAttributes& data = WindowAttributes()) = 0;
		virtual ~Window() {}

		virtual void onUpdate() = 0;

		virtual unsigned int getWidth() const = 0;
		virtual unsigned int getHeight() const = 0;

		// Window attributes
		virtual void setEventCallback(const EventCallbackFn& callback) = 0;
		virtual void setVSync(bool enabled) = 0;
		virtual bool isVSync() const = 0;

		virtual void* getNativeWindow() const = 0;

		static Window* create();
	};
}