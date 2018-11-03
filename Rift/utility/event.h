#pragma once
#ifndef _RIFT_EVENT_
#define _RIFT_EVENT_

#include <cstddef>

namespace rift {

	using EventFamily = std::size_t;

	// The BaseEvent class
	// Note:
	// Note: this class should not but subclassed directly as events need to be 'registered' see the Event class below
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

	class EventManager final {
	public:
	private:
	};

}

#endif // !_RIFT_EVENT_
