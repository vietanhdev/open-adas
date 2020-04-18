/*
 * Event.h
 *
 *  Created on: Sep 5, 2014
 *      Author: Cameron Karlsson
 *
 *  See the license file included with this source.
 */

#ifndef EVENT_H_
#define EVENT_H_

#include <list>
#include <functional>
#include <cstdint>



namespace nmea {


	template<class> class EventHandler;
	template<class> class Event;


	template<typename... Args>
	class EventHandler<void(Args...)>
	{
		friend Event<void(Args...)>;
	private:
		// Typenames
		typename Event<void(Args...)>::ListIterator _iterator;

		// Static members
		static uint64_t LastID;

		// Properties
		uint64_t ID;
		std::function<void(Args...)> handler;

		// Functions
		void _copy(const EventHandler& ref){
			if (&ref != this){
				_iterator = ref._iterator;
				handler = ref.handler;
				ID = ref.ID;
			}
		}

	public:
		// Typenames
		typedef void(*CFunctionPointer)(Args...);

		// Static members
		// (none)

		// Properties
		// (none)

		// Functions
		EventHandler(std::function<void(Args...)> h) : _iterator(), handler(h), ID(++LastID)
		{}

		EventHandler(const EventHandler& ref){
			_copy(ref);
		}

		virtual ~EventHandler(){};

		EventHandler& operator=(const EventHandler& ref){
			_copy(ref);
			return *this;
		}

		void operator() (Args... args){
			handler(args...);
		}

		bool operator==(const EventHandler& ref){
			return ID == ref.ID;
		}

		bool operator!=(const EventHandler& ref){
			return ID != ref.ID;
		}

		uint64_t getID(){
			return ID;
		}

		// Returns function pointer to the underlying function
		// or null if it's not a function but implements operator()
		CFunctionPointer* getFunctionPointer(){
			CFunctionPointer* ptr = handler.template target<CFunctionPointer>();
			return ptr;
		}
	};

	template<typename... Args>
	uint64_t EventHandler<void(Args...)>::LastID = 0;


	template <typename ... Args>
	class Event<void(Args...)>
	{
		friend EventHandler<void(Args...)>;
	private:
		// Typenames
		typedef typename std::list<EventHandler<void(Args...)>>::iterator ListIterator;

		// Static members
		// (none)

		// Properties
		std::list<EventHandler<void(Args...)>> handlers;

		//Functions
		void _copy(const Event& ref){
			if (&ref != this){
				handlers = ref.handlers;
			}
		};

		bool removeHandler(ListIterator handlerIter)	{
			if (handlerIter == handlers.end()){
				return false;
			}

			handlers.erase(handlerIter);
			return true;
		};

	public:
		// Typenames
		// (none)

		// Static members
		// (none)

		// Properties
		bool enabled;

		// Functions
		Event() : enabled(true)
		{}

		virtual ~Event() 
		{}

		Event(const Event& ref) 	{
			_copy(ref);
		}

		void call(Args... args)	{
			if (!enabled) { return; }
			for (auto h = handlers.begin(); h != handlers.end(); h++)
			{
				(*h)(args...);
			}
		}

		EventHandler<void(Args...)> registerHandler(EventHandler<void(Args...)> handler)	{
			bool found = false;
			for (auto h = handlers.begin(); h != handlers.end(); h++)
			{
				if ((*h) == handler)	{
					found = true;
					break;
				}
			}
			if (!found)
			{
				ListIterator itr = handlers.insert(handlers.end(), handler);
				handler._iterator = itr;
			}
			return handler;
		}

		EventHandler<void(Args...)> registerHandler(std::function<void(Args...)> handler)	{
			EventHandler<void(Args...)> wrapper(handler);
			ListIterator itr = handlers.insert(handlers.end(), wrapper);
			wrapper._iterator = itr;
			return wrapper;
		}

		bool removeHandler(EventHandler<void(Args...)>& handler)	{
			bool sts = removeHandler(handler._iterator);
			handler._iterator = handlers.end();
			return sts;
		};

		void clear(){
			for (auto h = handlers.begin(); h != handlers.end(); h++)
			{
				(*h)._iterator = handlers.end();
			}
			handlers.clear();
		};

		void operator ()(Args... args)													{ return call(args...); };
		EventHandler<void(Args...)> operator +=(EventHandler<void(Args...)> handler)	{ return registerHandler(handler); };
		EventHandler<void(Args...)> operator +=(std::function<void(Args...)> handler)	{ return registerHandler(handler); };
		bool operator -=(EventHandler<void(Args...)>& handler)							{ return removeHandler(handler); };
		bool operator -=(uint64_t handlerID)											{ return removeHandler(handlerID); };

		EventHandler<void(Args...)>& operator =(const EventHandler<void(Args...)>& ref){
			_copy(ref);
			return *this;
		};

	};



}

#endif /* EVENT_H_ */
