//! @file include/jenlib/events/EventDispatcher.h
//! @brief Event dispatcher for managing and processing events
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#ifndef INCLUDE_JENLIB_EVENTS_EVENTDISPATCHER_H_
#define INCLUDE_JENLIB_EVENTS_EVENTDISPATCHER_H_

#include <unordered_map>
#include <vector>
#include <memory>
#include "jenlib/events/EventTypes.h"

namespace jenlib::events {

//! @brief Event dispatcher for managing and processing events
//! @details
//! Provides a centralized event system for the jenlib library.
//! Supports event registration, dispatch, and processing with thread-safe
//! operations where supported by the platform.
class EventDispatcher {
public:
    //! @brief Register a callback for a specific event type
    //! @param event_type The type of event to register for
    //! @param callback The callback function to invoke
    //! @return EventId for unregistering the callback, or kInvalidEventId on failure
    static EventId register_callback(EventType event_type, EventCallback callback);
    
    //! @brief Unregister a callback by event ID
    //! @param event_id The ID returned from register_callback
    //! @return true if successfully unregistered, false if not found
    static bool unregister_callback(EventId event_id);
    
    //! @brief Unregister all callbacks for a specific event type
    //! @param event_type The event type to clear callbacks for
    //! @return Number of callbacks removed
    static std::size_t unregister_callbacks(EventType event_type);
    
    //! @brief Dispatch an event to all registered callbacks
    //! @param event The event to dispatch
    //! @return Number of callbacks invoked
    static std::size_t dispatch_event(const Event& event);
    
    //! @brief Process all pending events in the queue
    //! @return Number of events processed
    static std::size_t process_events();
    
    //! @brief Get the number of registered callbacks for an event type
    //! @param event_type The event type to query
    //! @return Number of registered callbacks
    static std::size_t get_callback_count(EventType event_type);
    
    //! @brief Get the total number of registered callbacks
    //! @return Total number of registered callbacks
    static std::size_t get_total_callback_count();
    
    //! @brief Clear all registered callbacks
    static void clear_all_callbacks();
    
    //! @brief Check if the event dispatcher is initialized
    //! @return true if initialized, false otherwise
    static bool is_initialized();
    
    //! @brief Initialize the event dispatcher (called automatically on first use)
    static void initialize();

private:
    //! @brief Internal callback entry structure
    struct CallbackEntry {
        EventId id;
        EventType type;
        EventCallback callback;
        
        CallbackEntry(EventId callback_id, EventType event_type, EventCallback cb)
            : id(callback_id), type(event_type), callback(std::move(cb)) {}
    };
    
    //! @brief Get the next available event ID
    static EventId get_next_event_id();
    
    //! @brief Internal initialization flag
    static bool initialized_;
    
    //! @brief Next available event ID
    static EventId next_event_id_;
    
    //! @brief Callback registry
    static std::unordered_map<EventId, std::unique_ptr<CallbackEntry>> callbacks_;
    
    //! @brief Event type to callback ID mapping
    static std::unordered_map<EventType, std::vector<EventId>> event_type_map_;
    
    //! @brief Event queue for pending events
    static std::vector<Event> event_queue_;
    
    //! @brief Maximum event queue size
    static constexpr std::size_t kMaxEventQueueSize = 32;
};

} // namespace jenlib::events

#endif // INCLUDE_JENLIB_EVENTS_EVENTDISPATCHER_H_
