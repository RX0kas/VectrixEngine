#pragma once

#include <concepts>
#include <cstddef>
#include <memory>
#include <new>
#include <utility>
#include <vector>

#include "Event.h"

/**
 * @file EventQueue.h
 * @brief Definition of EventQueue, where the events wait until the Application sends them
 * @ingroup events
 */

namespace Vectrix {
	/**
	 * @brief Holds the events posted during a frame until they are sent, at the start of the next one
	 *
	 * The Application owns one: the window posts the input to it, and Application::postEvent posts there too.
	 * The events are built in a buffer reused from frame to frame, so posting doesn't allocate once the
	 * buffer has grown to what a frame needs.
	 * @note Main thread only
	 * @ingroup events
	 */
	class EventQueue {
	public:
		EventQueue() = default;
		~EventQueue() {
			m_batches[0].clear();
			m_batches[1].clear();
		}
		EventQueue(const EventQueue&) = delete;
		EventQueue& operator=(const EventQueue&) = delete;

		/**
		 * @brief Build a T event in the queue
		 * @param args The arguments of T's constructor
		 */
		template<std::derived_from<Event> T, typename... Args> requires std::constructible_from<T, Args...>
		void post(Args&&... args) {
			static_assert(alignof(T) <= alignof(std::max_align_t), "Over-aligned events aren't supported");
			Batch& batch = m_batches[m_writing];
			void* memory = batch.arena.allocate(sizeof(T), alignof(T));
			batch.events.push_back(new (memory) T(std::forward<Args>(args)...));
		}

		/**
		 * @brief Give every event posted so far to a function, in the order they were posted, then drop them
		 *
		 * The events posted meanwhile (by the function) wait for the next call.
		 * @param send Called with each event
		 */
		template<std::invocable<Event&> Fn>
		void dispatch(Fn&& send) {
			Batch& batch = m_batches[m_writing];
			m_writing ^= 1;
			for (Event* event : batch.events)
				send(*event);
			batch.clear();
		}

		/// @brief Tell if no event is waiting
		[[nodiscard]] bool empty() const { return m_batches[m_writing].events.empty(); }
	private:
		// A bump allocator: what it gives back is freed all at once by reset. Past its capacity it allocates
		// apart, and grows by that much at the next reset
		class Arena {
		public:
			void* allocate(size_t size, size_t alignment) {
				if (!m_buffer) {
					m_buffer = std::make_unique<std::byte[]>(m_capacity);
				}
				const size_t offset = (m_used + alignment - 1) & ~(alignment - 1);
				if (offset + size > m_capacity) {
					m_overflowSize += size + alignment;
					return m_overflow.emplace_back(std::make_unique<std::byte[]>(size)).get();
				}
				m_used = offset + size;
				return m_buffer.get() + offset;
			}

			void reset() {
				if (!m_overflow.empty()) {
					m_capacity += m_overflowSize;
					m_buffer.reset(); // allocated again, bigger, on next use
					m_overflow.clear();
					m_overflowSize = 0;
				}
				m_used = 0;
			}
		private:
			static constexpr size_t k_initialCapacity = 16 * 1024;

			std::unique_ptr<std::byte[]> m_buffer;
			size_t m_capacity = k_initialCapacity;
			size_t m_used = 0;
			std::vector<std::unique_ptr<std::byte[]>> m_overflow;
			size_t m_overflowSize = 0;
		};

		struct Batch {
			Arena arena;
			std::vector<Event*> events;

			void clear() {
				for (Event* event : events)
					event->~Event();
				events.clear();
				arena.reset();
			}
		};

		// One batch takes the posts while the other is being sent
		Batch m_batches[2];
		int m_writing = 0;
	};
}
