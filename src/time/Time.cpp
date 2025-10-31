//! @file src/time/Time.cpp
//! @brief Time service implementation
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include "jenlib/time/Time.h"
#include <algorithm>
#include <cassert>
#include <utility>
#include "jenlib/time/drivers/NativeTimeDriver.h"

namespace jenlib::time {

// Static member definitions
bool Time::initialized_ = false;
TimerId Time::next_timer_id_ = 1;
std::array<TimerEntry, Time::kMaxTimers> Time::timers_;
std::size_t Time::timer_count_ = 0;
TimeDriver* Time::driver_ = nullptr;

TimerId Time::schedule_callback(std::uint32_t interval_ms, TimerCallback callback, bool repeat) {
    if (!callback || interval_ms == 0) {
        return kInvalidTimerId;
    }

    initialize();

    // Check if we have space for another timer
    if (timer_count_ >= kMaxTimers) {
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

    // Find available slot and create timer entry
    for (auto& timer : timers_) {
        if (timer.state == TimerState::kInactive) {
            timer = std::move(entry);
            ++timer_count_;
            return timer_id;
        }
    }

    return kInvalidTimerId;  //  Should not reach here if timer_count_ < kMaxTimers
}

bool Time::cancel_callback(TimerId timer_id) {
    if (timer_id == kInvalidTimerId) {
        return false;
    }

    for (auto& timer : timers_) {
        if (timer.id == timer_id && timer.state == TimerState::kActive) {
            timer.state = TimerState::kInactive;
            --timer_count_;
            return true;
        }
    }

    return false;
}

std::size_t Time::process_timers() {
    if (timer_count_ == 0) {
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
                timer.callback();
                ++fired_count;
            }

            // Handle repeat or mark as inactive
            if (timer.repeat) {
                // Reschedule for next interval
                timer.next_fire_time = current_time + timer.interval_ms;
                timer.state = TimerState::kActive;
            } else {
                // One-shot timer - mark as inactive
                timer.state = TimerState::kInactive;
                --timer_count_;
            }
        }
    }

    // Note: Inactive timers remain in the array but are not processed
    // This allows for efficient reuse of slots without expensive array operations

    return fired_count;
}

std::uint32_t Time::now() {
    if (!driver_) {
        // No-op when no driver is set - return 0
        return 0;
    }
    return driver_->now();
}

void Time::delay(std::uint32_t delay_ms) {
    if (!driver_) {
        // No-op when no driver is set - do nothing
        return;
    }
    driver_->delay(delay_ms);
}

std::size_t Time::get_active_timer_count() {
    return std::count_if(timers_.begin(), timers_.end(),
        [](const TimerEntry& entry) {
            return entry.state == TimerState::kActive;
        });
}

std::size_t Time::get_total_timer_count() {
    return timer_count_;
}

void Time::clear_all_timers() {
    for (auto& timer : timers_) {
        timer.state = TimerState::kInactive;
    }
    timer_count_ = 0;
}

bool Time::is_initialized() {
    return initialized_;
}

void Time::initialize() {
    if (!initialized_) {
        for (auto& timer : timers_) {
            timer.state = TimerState::kInactive;
        }
        timer_count_ = 0;
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

void Time::setDriver(TimeDriver* driver) noexcept {
    driver_ = driver;
}

TimeDriver* Time::getDriver() noexcept {
    return driver_;
}

}  // namespace jenlib::time
