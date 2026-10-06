#pragma once

#include <concepts>
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

#include "Event.h"

/**
 * @file EventListener.h
 * @brief Definition of EventListener, which holds the handlers subscribed to events
 * @ingroup events
 */

namespace Vectrix {
	/**
	 * @brief Identifies one subscription, to remove it with EventListener::unsubscribe
	 * @ingroup events
	 */
	using SubscriptionId = uint64_t;

	/**
	 * @brief A callable that can handle a T event
	 *
	 * It takes a `const T&`, or nothing, and returns `bool` (true consumes the event) or `void` (never
	 * consumes it).
	 * @ingroup events
	 */
	template<typename F, typename T>
	concept EventHandler = std::derived_from<T, Event> && (std::invocable<F&, const T&> || std::invocable<F&>);

	/**
	 * @brief Receives the events it subscribed to
	 *
	 * Layer and Application are listeners: subscribe once, from the constructor or OnAttach, and the handler
	 * is called for every matching event until it is unsubscribed or the listener destroyed.
	 * @code
	 * MyLayer::MyLayer() : Layer("MyLayer") {
	 *     subscribe<KeyPressedEvent>([this](const KeyPressedEvent& e) {
	 *         if (e.key != VC_KEY_ESCAPE) return false; // let the layers below get it
	 *         togglePause();
	 *         return true;                              // consumed: the layers below don't get it
	 *     });
	 *     subscribe<WindowResizeEvent>(&MyLayer::onResize);  // a member function works too
	 *     subscribe<WindowFocusEvent>([this] { m_paused = true; }); // the event can be left out
	 * }
	 * @endcode
	 *
	 * The engine sends the events once per frame, before Layer::OnUpdate, from the top of the layer stack
	 * down (ImGui, the overlays from the last pushed, the layers from the last pushed) and then to the
	 * Application. Within a listener the handlers run in subscription order. Once a handler consumes the
	 * event, nothing after it receives it.
	 *
	 * Subscribing to Event itself (`subscribe<Event>`) receives every event.
	 * @note Handlers may subscribe and unsubscribe, even themselves, while an event is being sent.
	 *       A handler added then only receives the next events
	 * @see Application::postEvent
	 * @ingroup events
	 */
	class EventListener {
	public:
		EventListener() = default;
		virtual ~EventListener() = default;
		// The handlers usually capture this
		EventListener(const EventListener&) = delete;
		EventListener& operator=(const EventListener&) = delete;

		/**
		 * @brief Call a handler for every T event
		 * @tparam T The event class, or Event to receive every event
		 * @param handler A callable taking a `const T&`, or nothing, and returning bool (true consumes the
		 *                event) or void (never consumes it)
		 * @return The id to give unsubscribe
		 */
		template<std::derived_from<Event> T, typename F> requires EventHandler<std::decay_t<F>, T>
		SubscriptionId subscribe(F&& handler) {
			return add(eventTypeId<T>(), [h = std::forward<F>(handler)](const Event& event) mutable {
				return invokeHandler(h, static_cast<const T&>(event));
			});
		}

		/**
		 * @brief Call a member function of this listener for every T event
		 * @param method A member function of the listener's class, taking a `const T&` and returning bool
		 *               (true consumes the event) or void (never consumes it)
		 * @return The id to give unsubscribe
		 */
		template<std::derived_from<Event> T, std::derived_from<EventListener> C, typename R>
			requires std::is_void_v<R> || std::convertible_to<R, bool>
		SubscriptionId subscribe(R (C::*method)(const T&)) {
			C* self = static_cast<C*>(this);
			return subscribe<T>([self, method](const T& event) { return (self->*method)(event); });
		}

		/// @copydoc subscribe(R (C::*)(const T&))
		template<std::derived_from<Event> T, std::derived_from<EventListener> C, typename R>
			requires std::is_void_v<R> || std::convertible_to<R, bool>
		SubscriptionId subscribe(R (C::*method)(const T&) const) {
			const C* self = static_cast<const C*>(this);
			return subscribe<T>([self, method](const T& event) { return (self->*method)(event); });
		}

		/**
		 * @brief Remove one subscription
		 * @param id The value subscribe returned; an id already removed is ignored
		 */
		void unsubscribe(SubscriptionId id) {
			for (const auto& subscription : m_subscriptions)
				if (subscription->id == id) remove(*subscription);
			cleanUp();
		}

		/// @brief Remove every subscription to T events (not those to every event)
		template<std::derived_from<Event> T>
		void unsubscribe() {
			for (const auto& subscription : m_subscriptions)
				if (subscription->type == eventTypeId<T>()) remove(*subscription);
			cleanUp();
		}

		/// @brief Remove every subscription of this listener
		void unsubscribeAll() {
			for (const auto& subscription : m_subscriptions) remove(*subscription);
			cleanUp();
		}
	private:
		friend class Application;

		struct Subscription {
			SubscriptionId id;
			EventTypeId type;
			std::function<bool(const Event&)> handler;
			bool active = true;
		};

		template<typename T, typename F>
		static bool invokeHandler(F& handler, const T& event) {
			if constexpr (std::invocable<F&, const T&>) {
				if constexpr (std::is_void_v<std::invoke_result_t<F&, const T&>>) {
					std::invoke(handler, event);
					return false;
				} else {
					return static_cast<bool>(std::invoke(handler, event));
				}
			} else {
				if constexpr (std::is_void_v<std::invoke_result_t<F&>>) {
					std::invoke(handler);
					return false;
				} else {
					return static_cast<bool>(std::invoke(handler));
				}
			}
		}

		SubscriptionId add(EventTypeId type, std::function<bool(const Event&)> handler) {
			const SubscriptionId id = m_nextId++;
			m_subscriptions.push_back(std::make_unique<Subscription>(Subscription{id, type, std::move(handler)}));
			return id;
		}

		// A handler that is running can't be destroyed: it is only deactivated, and erased once no event is
		// being sent anymore
		void remove(Subscription& subscription) {
			subscription.active = false;
			m_hasInactive = true;
		}

		void cleanUp() {
			if (m_sending > 0 || !m_hasInactive) return;
			std::erase_if(m_subscriptions, [](const auto& subscription) { return !subscription->active; });
			m_hasInactive = false;
		}

		// Called by the Application for every event, see the class description for the order
		void notify(Event& event) {
			++m_sending;
			// The subscriptions are held by pointer and only counted up to here: one added by a handler moves the
			// vector but not the subscription being run, and waits for the next event
			const size_t count = m_subscriptions.size();
			for (size_t i = 0; i < count && !event.m_handled; ++i) {
				Subscription& subscription = *m_subscriptions[i];
				if (!subscription.active) continue;
				if (subscription.type == event.getType() || subscription.type == eventTypeId<Event>())
					event.m_handled = subscription.handler(event);
			}
			--m_sending;
			cleanUp();
		}

		std::vector<std::unique_ptr<Subscription>> m_subscriptions;
		SubscriptionId m_nextId = 1;
		int m_sending = 0;
		bool m_hasInactive = false;
	};
}
