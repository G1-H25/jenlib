//! @file include/jenlib/time/TimeDriver.h
//! @brief Abstract interface for time drivers
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_TIME_TIMEDRIVER_H_
#define INCLUDE_JENLIB_TIME_TIMEDRIVER_H_

#include "jenlib/time/TimeTypes.h"

namespace jenlib::time {

//! @brief Abstract interface for time drivers
//! @details
//! Defines the contract that all time drivers must implement.
//! Provides platform-agnostic time functionality for the Time service.
class TimeDriver {
 public:
    virtual ~TimeDriver() = default;

    //! @brief Get current time in milliseconds
    //! @return Current time in milliseconds since system start
    virtual std::uint32_t now() = 0;

    //! @brief Block for the specified number of milliseconds
    //! @param delay_ms Number of milliseconds to delay
    virtual void delay(std::uint32_t delay_ms) = 0;

    virtual bool has_overflowed(std::uint32_t time_value) noexcept = 0;

    virtual std::uint32_t time_difference(std::uint32_t current_time, std::uint32_t previous_time) noexcept = 0;
};

}  // namespace jenlib::time

#endif  // INCLUDE_JENLIB_TIME_TIMEDRIVER_H_
