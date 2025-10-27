//! @file include/jenlib/time/drivers/ArduinoTimeDriver.h
//! @brief Arduino-specific time driver implementation
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_TIME_DRIVERS_ARDUINOTIMEDRIVER_H_
#define INCLUDE_JENLIB_TIME_DRIVERS_ARDUINOTIMEDRIVER_H_

#include "jenlib/time/TimeDriver.h"
#include "jenlib/time/TimeTypes.h"

#ifdef ARDUINO

namespace jenlib::time {

//! @brief Arduino-specific time driver
//! @details
//! Provides time functionality using Arduino's millis() function.
//! Handles timer overflow (49.7 days) and provides non-blocking timer processing.
class ArduinoTimeDriver : public TimeDriver {
 public:
    //! @brief Get current time in milliseconds using Arduino millis()
    //! @return Current time in milliseconds
    std::uint32_t now() override;

    //! @brief Delay execution for specified milliseconds using Arduino delay()
    //! @param delay_ms Delay duration in milliseconds
    void delay(std::uint32_t delay_ms) override;

    //! @brief Static versions for backward compatibility
    static std::uint32_t now_static();
    static void delay_static(std::uint32_t delay_ms);

    //! @brief Check if a time value has overflowed (for 32-bit millis())
    //! @param time_value The time value to check
    //! @return true if the time value indicates an overflow condition
    static bool has_overflowed(std::uint32_t time_value);

    //! @brief Calculate time difference handling overflow
    //! @param current_time Current time value
    //! @param previous_time Previous time value
    //! @return Time difference in milliseconds
    static std::uint32_t time_difference(std::uint32_t current_time, std::uint32_t previous_time);

 private:
    //! @brief Last known time value for overflow detection
    static std::uint32_t last_time_;

    //! @brief Overflow count for extended time tracking
    static std::uint32_t overflow_count_;
};

}  // namespace jenlib::time

#else
// Fallback implementation for non-Arduino platforms
namespace jenlib::time {

class ArduinoTimeDriver {
 public:
    ArduinoTimeDriver() = delete;
};

}  // namespace jenlib::time

#endif  // ARDUINO

#endif  // INCLUDE_JENLIB_TIME_DRIVERS_ARDUINOTIMEDRIVER_H_

