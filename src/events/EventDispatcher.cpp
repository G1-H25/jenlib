//! @file src/events/EventDispatcher.cpp
//! @brief Event dispatcher implementation
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include "jenlib/events/EventDispatcher.h"
#include <algorithm>
#include <cassert>

namespace jenlib::events {

// Static member definitions
bool EventDispatcher::initialized_ = false;
EventId EventDispatcher::next_event_id_ = 1;
std::unordered_map<EventId, std::unique_ptr<EventDispatcher::CallbackEntry>> EventDispatcher::callbacks_;
std::unordered_map<EventType, std::vector<EventId>> EventDispatcher::event_type_map_;
std::vector<Event> EventDispatcher::event_queue_;

EventId EventDispatcher::register_callback(EventType event_type, EventCallback callback) {
    if (!callback) {
        return kInvalidEventId;
    }
    
    initialize();
    
    EventId event_id = get_next_event_id();
    if (event_id == kInvalidEventId) {
        return kInvalidEventId;
    }
    
    // Create callback entry
    auto entry = std::make_unique<CallbackEntry>(event_id, event_type, std::move(callback));
    
    // Store callback
    callbacks_[event_id] = std::move(entry);
    
    // Add to event type mapping
    event_type_map_[event_type].push_back(event_id);
    
    return event_id;
}

bool EventDispatcher::unregister_callback(EventId event_id) {
    if (event_id == kInvalidEventId) {
        return false;
    }
    
    auto it = callbacks_.find(event_id);
    if (it == callbacks_.end()) {
        return false;
    }
    
    EventType event_type = it->second->type;
    
    // Remove from event type mapping
    auto type_it = event_type_map_.find(event_type);
    if (type_it != event_type_map_.end()) {
        auto& id_list = type_it->second;
        id_list.erase(std::remove(id_list.begin(), id_list.end(), event_id), id_list.end());
        
        // Remove empty event type entry
        if (id_list.empty()) {
            event_type_map_.erase(type_it);
        }
    }
    
    // Remove callback
    callbacks_.erase(it);
    
    return true;
}

std::size_t EventDispatcher::unregister_callbacks(EventType event_type) {
    auto it = event_type_map_.find(event_type);
    if (it == event_type_map_.end()) {
        return 0;
    }
    
    std::size_t count = 0;
    const auto& id_list = it->second;
    
    // Remove all callbacks for this event type
    for (EventId event_id : id_list) {
        if (unregister_callback(event_id)) {
            ++count;
        }
    }
    
    return count;
}

std::size_t EventDispatcher::dispatch_event(const Event& event) {
    initialize();
    
    // Check if event queue is full
    if (event_queue_.size() >= kMaxEventQueueSize) {
        // Remove oldest event (simple FIFO behavior)
        event_queue_.erase(event_queue_.begin());
    }
    
    // Add event to queue
    event_queue_.push_back(event);
    
    return 1;
}

std::size_t EventDispatcher::process_events() {
    if (event_queue_.empty()) {
        return 0;
    }
    
    std::size_t processed_count = 0;
    
    // Process all events in the queue
    for (const Event& event : event_queue_) {
        auto it = event_type_map_.find(event.type);
        if (it != event_type_map_.end()) {
            const auto& id_list = it->second;
            
            // Invoke all callbacks for this event type
            for (EventId event_id : id_list) {
                auto callback_it = callbacks_.find(event_id);
                if (callback_it != callbacks_.end() && callback_it->second->callback) {
                    try {
                        callback_it->second->callback(event);
                        ++processed_count;
                    } catch (...) {
                        // Callback exception - continue processing other callbacks
                        // In a production system, you might want to log this
                    }
                }
            }
        }
    }
    
    // Clear the processed events
    event_queue_.clear();
    
    return processed_count;
}

std::size_t EventDispatcher::get_callback_count(EventType event_type) {
    auto it = event_type_map_.find(event_type);
    return (it != event_type_map_.end()) ? it->second.size() : 0;
}

std::size_t EventDispatcher::get_total_callback_count() {
    return callbacks_.size();
}

void EventDispatcher::clear_all_callbacks() {
    callbacks_.clear();
    event_type_map_.clear();
    event_queue_.clear();
}

bool EventDispatcher::is_initialized() {
    return initialized_;
}

void EventDispatcher::initialize() {
    if (!initialized_) {
        callbacks_.clear();
        event_type_map_.clear();
        event_queue_.clear();
        next_event_id_ = 1;
        initialized_ = true;
    }
}

EventId EventDispatcher::get_next_event_id() {
    if (next_event_id_ == kInvalidEventId) {
        // Handle ID overflow - in practice, this is unlikely to happen
        // For embedded systems, we might want to implement ID recycling
        return kInvalidEventId;
    }
    
    return next_event_id_++;
}

} // namespace jenlib::events
