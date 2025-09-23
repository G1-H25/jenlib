//! @file include/jenlib/time/Time.h
//! @brief Time service interface for jenlib
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#ifndef INCLUDE_JENLIB_TIME_TIME_H_
#define INCLUDE_JENLIB_TIME_TIME_H_

#include "jenlib/time/TimeDriver.h"
#include "jenlib/time/TimeTypes.h"

//! @namespace jenlib::time
//! @brief Abstracting time handling away from hardware bound libraries
namespace jenlib::time {

//! @brief Time service for managing timers and time operations
//! @details
//! Provides a platform-abstracted time service for the jenlib library.
//! Supports timer scheduling, cancellation, and processing across different
//! platforms (Arduino, ESP-IDF, native).
class Time {
public:
    //! @brief Schedule a timer callback
    //! @param interval_ms Timer interval in milliseconds
    //! @param callback Function to call when timer expires
    //! @param repeat Whether the timer should repeat
    //! @return TimerId for canceling the timer, or kInvalidTimerId on failure
    static TimerId schedule_callback(std::uint32_t interval_ms, TimerCallback callback, bool repeat = false);

    //! @brief Cancel a scheduled timer
    //! @param timer_id The timer ID returned from schedule_callback
    //! @return true if successfully canceled, false if not found
    static bool cancel_callback(TimerId timer_id);

    //! @brief Process all active timers
    //! @return Number of timers that fired
    static std::size_t process_timers();

    //! @brief Get current time in milliseconds (platform-specific)
    //! @return Current time in milliseconds
    static std::uint32_t now();

    //! @brief Delay execution for specified milliseconds
    //! @param delay_ms Delay duration in milliseconds
    static void delay(std::uint32_t delay_ms);

    //! @brief Get the number of active timers
    //! @return Number of active timers
    static std::size_t get_active_timer_count();

    //! @brief Get the total number of timers (active + inactive)
    //! @return Total number of timers
    static std::size_t get_total_timer_count();

    //! @brief Clear all timers
    static void clear_all_timers();

    //! @brief Check if the time service is initialized
    //! @return true if initialized, false otherwise
    static bool is_initialized();

    //! @brief Initialize the time service (called automatically on first use)
    static void initialize();

    //! @brief Set the time driver for dependency injection
    //! @param driver Pointer to the time driver implementation
    static void setDriver(TimeDriver* driver) noexcept;

    //! @brief Get the current time driver
    //! @return Pointer to the current time driver, or nullptr if none set
    static TimeDriver* getDriver() noexcept;

private:
    //! @brief Get the next available timer ID
    static TimerId get_next_timer_id();

    //! @brief Internal initialization flag
    static bool initialized_;

    //! @brief Next available timer ID
    static TimerId next_timer_id_;

    //! @brief Maximum number of timers
    static constexpr std::size_t kMaxTimers = 16;

    //! @brief Timer storage (static allocation)
    static std::array<TimerEntry, kMaxTimers> timers_;

    //! @brief Current number of active timers
    static std::size_t timer_count_;

    //! @brief Current time driver (dependency injection)
    static TimeDriver* driver_;
};

//! @brief Convenience function to schedule a repeating timer
//! @param interval_ms Timer interval in milliseconds
//! @param callback Function to call when timer expires
//! @return TimerId for canceling the timer, or kInvalidTimerId on failure
inline TimerId schedule_repeating_timer(std::uint32_t interval_ms, TimerCallback callback) {
    return Time::schedule_callback(interval_ms, std::move(callback), true);
}

//! @brief Convenience function to schedule a one-shot timer
//! @param delay_ms Delay duration in milliseconds
//! @param callback Function to call when timer expires
//! @return TimerId for canceling the timer, or kInvalidTimerId on failure
inline TimerId schedule_one_shot(std::uint32_t delay_ms, TimerCallback callback) {
    return Time::schedule_callback(delay_ms, std::move(callback), false);
}

} // namespace jenlib::time

#endif // INCLUDE_JENLIB_TIME_TIME_H_

