//! @file src/time/Time.cpp
//! @brief Time service implementation
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include "jenlib/time/Time.h"
#include "jenlib/time/drivers/NativeTimeDriver.h"
#include <algorithm>
#include <cassert>

namespace jenlib::time {

// Static member definitions
bool Time::initialized_ = false;
TimerId Time::next_timer_id_ = 1;
std::vector<TimerEntry> Time::timers_;

TimerId Time::schedule_callback(std::uint32_t interval_ms, TimerCallback callback, bool repeat) {
    if (!callback || interval_ms == 0) {
        return kInvalidTimerId;
    }
    
    initialize();
    
    // Check if we've reached the maximum number of timers
    if (timers_.size() >= kMaxTimers) {
        return kInvalidTimerId;
    }
    
    TimerId timer_id = get_next_timer_id();
    if (timer_id == kInvalidTimerId) {
        return kInvalidTimerId;
    }
    
    std::uint32_t current_time = now();
    std::uint32_t fire_time = current_time + interval_ms;
    
    // Create timer entry
    TimerEntry entry(timer_id, interval_ms, fire_time, std::move(callback), repeat);
    
    // Add to timer list
    timers_.push_back(std::move(entry));
    
    return timer_id;
}

bool Time::cancel_callback(TimerId timer_id) {
    if (timer_id == kInvalidTimerId) {
        return false;
    }
    
    auto it = std::find_if(timers_.begin(), timers_.end(),
        [timer_id](const TimerEntry& entry) {
            return entry.id == timer_id;
        });
    
    if (it != timers_.end()) {
        it->state = TimerState::kInactive;
        return true;
    }
    
    return false;
}

std::size_t Time::process_timers() {
    if (timers_.empty()) {
        return 0;
    }
    
    std::uint32_t current_time = now();
    std::size_t fired_count = 0;
    
    // Process all active timers
    for (auto& timer : timers_) {
        if (timer.state == TimerState::kActive && current_time >= timer.next_fire_time) {
            // Timer has expired
            timer.state = TimerState::kExpired;
            
            // Invoke callback
            if (timer.callback) {
                try {
                    timer.callback();
                    ++fired_count;
                } catch (...) {
                    // Callback exception - continue processing other timers
                    // In a production system, you might want to log this
                }
            }
            
            // Handle repeat or mark as inactive
            if (timer.repeat) {
                // Reschedule for next interval
                timer.next_fire_time = current_time + timer.interval_ms;
                timer.state = TimerState::kActive;
            } else {
                // One-shot timer - mark as inactive
                timer.state = TimerState::kInactive;
            }
        }
    }
    
    // Remove inactive timers to free up space
    timers_.erase(
        std::remove_if(timers_.begin(), timers_.end(),
            [](const TimerEntry& entry) {
                return entry.state == TimerState::kInactive;
            }),
        timers_.end()
    );
    
    return fired_count;
}

std::uint32_t Time::now() {
    return drivers::NativeTimeDriver::now();
}

void Time::delay(std::uint32_t delay_ms) {
    drivers::NativeTimeDriver::delay(delay_ms);
}

std::size_t Time::get_active_timer_count() {
    return std::count_if(timers_.begin(), timers_.end(),
        [](const TimerEntry& entry) {
            return entry.state == TimerState::kActive;
        });
}

std::size_t Time::get_total_timer_count() {
    return timers_.size();
}

void Time::clear_all_timers() {
    timers_.clear();
}

bool Time::is_initialized() {
    return initialized_;
}

void Time::initialize() {
    if (!initialized_) {
        timers_.clear();
        next_timer_id_ = 1;
        initialized_ = true;
    }
}

TimerId Time::get_next_timer_id() {
    if (next_timer_id_ == kInvalidTimerId) {
        // Handle ID overflow - in practice, this is unlikely to happen
        // For embedded systems, we might want to implement ID recycling
        return kInvalidTimerId;
    }
    
    return next_timer_id_++;
}

} // namespace jenlib::time
