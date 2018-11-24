#pragma once

#include <vector>
#include <cstddef>
#include "internal/signal.h"
#include "internal/noncopyable.h"

namespace rift {

	using EventFamily = std::size_t;

	// The BaseComponent class
	// Note: this class should not but subclassed directly as events need to be registered. See the Event class below.
	class BaseEvent {
	public:

		virtual ~BaseEvent() = default;

	protected:
		
		static EventFamily m_family;
	};

	class EventManager;

	// The Event class
	// Classes that are meant to be events must inherit from this class for registration as an 'event'
	template <class Derived> 
	class Event : public BaseEvent {
	public:
		
		virtual ~Event() = default;

	private:
		
		friend class EventManager;

		static EventFamily family() noexcept {
			static EventFamily event_family = m_family++;
			return event_family;
		}

	};

	class BaseSubscriber {
	public:
	private:
	};

	template <class Derived>
	class Subscriber : public BaseSubscriber {
	public:
	};

	class EventManager final : rift::impl::NonCopyable {

		template <class Receiver, class EventType>
		struct EventCallback {};

	public:



	private:
		
	};

}
