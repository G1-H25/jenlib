//! @file src/events/EventDispatcher.cpp
//! @brief Event dispatcher implementation
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include "jenlib/events/EventDispatcher.h"
#include <algorithm>
#include <cassert>
#include <utility>

namespace jenlib::events {

// Static member definitions
bool EventDispatcher::initialized_ = false;
EventId EventDispatcher::next_event_id_ = 1;
std::array<EventDispatcher::CallbackEntry, EventDispatcher::kMaxCallbacks> EventDispatcher::callbacks_;
std::array<Event, EventDispatcher::kMaxEventQueueSize> EventDispatcher::event_queue_;
std::size_t EventDispatcher::queue_size_ = 0;
std::size_t EventDispatcher::queue_head_ = 0;

EventId EventDispatcher::register_callback(EventType event_type, EventCallback callback) {
    if (!callback) {
        return kInvalidEventId;
    }

    initialize();

    EventId event_id = get_next_event_id();
    if (event_id == kInvalidEventId) {
        return kInvalidEventId;
    }

    // Find available slot
    CallbackEntry* entry = find_available_slot();
    if (!entry) {
        return kInvalidEventId;  // No available slots
    }

    // Store callback in available slot
    *entry = CallbackEntry(event_id, event_type, std::move(callback));

    return event_id;
}

bool EventDispatcher::unregister_callback(EventId event_id) {
    if (event_id == kInvalidEventId) {
        return false;
    }

    CallbackEntry* entry = find_callback_entry(event_id);
    if (!entry || !entry->active) {
        return false;
    }

    // Clear the callback entry
    entry->clear();

    return true;
}

std::size_t EventDispatcher::unregister_callbacks(EventType event_type) {
    std::size_t count = 0;

    // Find and remove all callbacks for this event type
    for (auto& entry : callbacks_) {
        if (entry.active && entry.type == event_type) {
            entry.clear();
            ++count;
        }
    }

    return count;
}

EventEnqueueResult EventDispatcher::dispatch_event(const Event& event, Event* evicted_event /* = nullptr */) {
    initialize();

    auto result = EventEnqueueResult::Enqueued;

    // If full, evict the oldest by advancing head and decreasing size
    if (queue_size_ >= kMaxEventQueueSize) {
        if (evicted_event) {
            *evicted_event = event_queue_[queue_head_];
        }
        queue_head_ = (queue_head_ + 1) % kMaxEventQueueSize;
        --queue_size_;
        result = EventEnqueueResult::EnqueuedWithEviction;
    }

    // Compute tail position relative to head and size, with wrap-around
    const std::size_t tail = (queue_head_ + queue_size_) % kMaxEventQueueSize;
    event_queue_[tail] = event;
    ++queue_size_;

    return result;
}

std::size_t EventDispatcher::process_events() {
    if (queue_size_ == 0) {
        return 0;
    }

    std::size_t processed_count = 0;

    // Process all events in the queue using range-based for loop
    for (const Event& event : event_queue_range()) {
        // Find all callbacks for this event type
        for (const auto& entry : callbacks_) {
            if (entry.active && entry.type == event.type && entry.callback) {
                try {
                    entry.callback(event);
                    ++processed_count;
                } catch (...) {
                    // Callback exception - continue processing other callbacks
                    // In Arduino, exceptions should be avoided entirely
                }
            }
        }
    }

    // Clear the processed events
    queue_size_ = 0;
    queue_head_ = 0;

    return processed_count;
}

std::size_t EventDispatcher::get_callback_count(EventType event_type) {
    std::size_t count = 0;
    for (const auto& entry : callbacks_) {
        if (entry.active && entry.type == event_type) {
            ++count;
        }
    }
    return count;
}

std::size_t EventDispatcher::get_total_callback_count() {
    std::size_t count = 0;
    for (const auto& entry : callbacks_) {
        if (entry.active) {
            ++count;
        }
    }
    return count;
}

void EventDispatcher::clear_all_callbacks() {
    for (auto& entry : callbacks_) {
        entry.clear();
    }
    queue_size_ = 0;
    queue_head_ = 0;
}

bool EventDispatcher::is_initialized() {
    return initialized_;
}

void EventDispatcher::initialize() {
    if (!initialized_) {
        for (auto& entry : callbacks_) {
            entry.clear();
        }
        queue_size_ = 0;
        queue_head_ = 0;
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

EventDispatcher::CallbackEntry* EventDispatcher::find_available_slot() {
    for (auto& entry : callbacks_) {
        if (!entry.active) {
            return &entry;
        }
    }
    return nullptr;  // No available slots
}

EventDispatcher::CallbackEntry* EventDispatcher::find_callback_entry(EventId event_id) {
    for (auto& entry : callbacks_) {
        if (entry.active && entry.id == event_id) {
            return &entry;
        }
    }
    return nullptr;  // Not found
}

EventDispatcher::CircularBufferIterator EventDispatcher::event_queue_begin() {
    return CircularBufferIterator(event_queue_, queue_head_, queue_size_, 0);
}

EventDispatcher::CircularBufferIterator EventDispatcher::event_queue_end() {
    return CircularBufferIterator(event_queue_, queue_head_, queue_size_, queue_size_);
}

}  // namespace jenlib::events

