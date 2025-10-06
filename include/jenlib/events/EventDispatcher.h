//! @file include/jenlib/events/EventDispatcher.h
//! @brief Event dispatcher for managing and processing events
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_EVENTS_EVENTDISPATCHER_H_
#define INCLUDE_JENLIB_EVENTS_EVENTDISPATCHER_H_

#include <array>
#include <utility>
#include "jenlib/events/EventTypes.h"

namespace jenlib::events {

//! @brief Result of the event enqueue operation
enum class EventEnqueueResult : std::uint8_t {
    Enqueued,
    EnqueuedWithEviction
};


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

    //! @brief Dispatch an event to the processing queue
    //! @param event The event to dispatch
    //! @param evicted_event Optional pointer to an event that was evicted to make room for the new event
    //! @return Result of the enqueue operation
    static EventEnqueueResult dispatch_event(const Event& event, Event* evicted_event /* = nullptr */);

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
        bool active;

        CallbackEntry() : id(kInvalidEventId), type(EventType::kCustom), callback(nullptr), active(false) {}

        CallbackEntry(EventId callback_id, EventType event_type, EventCallback cb)
            : id(callback_id), type(event_type), callback(std::move(cb)), active(true) {}

        void clear() {
            id = kInvalidEventId;
            type = EventType::kCustom;
            callback = nullptr;
            active = false;
        }
    };

    //! @brief Get the next available event ID
    static EventId get_next_event_id();

    //! @brief Find an available callback slot
    static CallbackEntry* find_available_slot();

    //! @brief Find callback entry by ID
    static CallbackEntry* find_callback_entry(EventId event_id);

    //! @brief Internal initialization flag
    static bool initialized_;

    //! @brief Next available event ID
    static EventId next_event_id_;

    //! @brief Maximum number of callbacks (static allocation)
    static constexpr std::size_t kMaxCallbacks = 16;

    //! @brief Maximum event queue size
    static constexpr std::size_t kMaxEventQueueSize = 32;

    //! @brief Static callback storage (no dynamic allocation)
    static std::array<CallbackEntry, kMaxCallbacks> callbacks_;

    //! @brief Event queue for pending events (circular buffer)
    static std::array<Event, kMaxEventQueueSize> event_queue_;

    //! @brief Current queue size
    static std::size_t queue_size_;

    //! @brief Current queue head index
    static std::size_t queue_head_;

    //! @brief Circular buffer iterator for events
    class CircularBufferIterator {
     public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Event;
        using difference_type = std::ptrdiff_t;
        using pointer = Event*;
        using reference = Event&;

        CircularBufferIterator(std::array<Event, kMaxEventQueueSize>& buffer,
                              std::size_t head, std::size_t size, std::size_t pos)
            : buffer_(buffer), head_(head), size_(size), pos_(pos) {}

        reference operator*() const {
            return buffer_[(head_ + pos_) % kMaxEventQueueSize];
        }

        pointer operator->() const {
            return &buffer_[(head_ + pos_) % kMaxEventQueueSize];
        }

        CircularBufferIterator& operator++() {
            ++pos_;
            return *this;
        }

        CircularBufferIterator operator++(int) {
            CircularBufferIterator temp = *this;
            ++pos_;
            return temp;
        }

        bool operator==(const CircularBufferIterator& other) const {
            return pos_ == other.pos_;
        }

        bool operator!=(const CircularBufferIterator& other) const {
            return pos_ != other.pos_;
        }

     private:
        std::array<Event, kMaxEventQueueSize>& buffer_;
        std::size_t head_;
        std::size_t size_;
        std::size_t pos_;
    };

    //! @brief Get begin iterator for event queue
    static CircularBufferIterator event_queue_begin();

    //! @brief Get end iterator for event queue
    static CircularBufferIterator event_queue_end();

    //! @brief Get range for event queue (for range-based for loops)
    static auto event_queue_range() {
        struct Range {
            CircularBufferIterator begin() const { return EventDispatcher::event_queue_begin(); }
            CircularBufferIterator end() const { return EventDispatcher::event_queue_end(); }
        };
        return Range{};
    }
};

}  // namespace jenlib::events

#endif  // INCLUDE_JENLIB_EVENTS_EVENTDISPATCHER_H_

