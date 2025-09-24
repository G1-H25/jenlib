//! @file include/jenlib/time/TimeTypes.h
//! @brief Time service types and structures for jenlib
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_TIME_TIMETYPES_H_
#define INCLUDE_JENLIB_TIME_TIMETYPES_H_

#include <cstdint>
#include <functional>
#include <utility>

namespace jenlib::time {

//! @brief Timer ID type for identifying timers
using TimerId = std::uint32_t;

//! @brief Invalid timer ID constant
constexpr TimerId kInvalidTimerId = 0;

//! @brief Timer callback function type
using TimerCallback = std::function<void()>;

//! @brief Timer state enumeration
enum class TimerState : std::uint8_t {
    kInactive = 0,  //!< Timer is not active
    kActive = 1,    //!< Timer is active and running
    kExpired = 2    //!< Timer has expired and needs processing
};

//! @brief Timer entry structure for internal timer management
struct TimerEntry {
    TimerId id;                 //!< Unique timer identifier
    std::uint32_t interval_ms;  //!< Timer interval in milliseconds
    std::uint32_t next_fire_time; //!< Next fire time (platform-specific)
    TimerCallback callback;     //!< Callback function to invoke
    bool repeat;                //!< Whether timer repeats
    TimerState state;           //!< Current timer state

    //! @brief Default constructor
    TimerEntry()
        : id(kInvalidTimerId)
        , interval_ms(0)
        , next_fire_time(0)
        , repeat(false)
        , state(TimerState::kInactive) {}

    //! @brief Constructor with parameters
    TimerEntry(TimerId timer_id, std::uint32_t interval, std::uint32_t fire_time,
               TimerCallback cb, bool should_repeat)
        : id(timer_id)
        , interval_ms(interval)
        , next_fire_time(fire_time)
        , callback(std::move(cb))
        , repeat(should_repeat)
        , state(TimerState::kActive) {}
};

}  // namespace jenlib::time

#endif // INCLUDE_JENLIB_TIME_TIMETYPES_H_

