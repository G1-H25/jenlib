//! @file src/time/drivers/ArduinoTimeDriver.cpp
//! @brief Arduino-specific time driver implementation
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)


#ifdef ARDUINO
#include "jenlib/time/drivers/ArduinoTimeDriver.h"
#include <Arduino.h>

//! @namespace jenlib::time
namespace jenlib::time {

// Static member definitions
std::uint32_t ArduinoTimeDriver::last_time_ = 0;
std::uint32_t ArduinoTimeDriver::overflow_count_ = 0;

std::uint32_t ArduinoTimeDriver::now() {
    std::uint32_t current_time = millis();

    // Check for overflow (millis() wraps around after ~49.7 days)
    if (current_time < last_time_) {
        ++overflow_count_;
    }
    last_time_ = current_time;

    return current_time;
}

void ArduinoTimeDriver::delay(std::uint32_t delay_ms) {
    ::delay(delay_ms);
}

bool ArduinoTimeDriver::has_overflowed(std::uint32_t time_value) {
    return time_value < last_time_;
}

std::uint32_t ArduinoTimeDriver::time_difference(std::uint32_t current_time, std::uint32_t previous_time) {
    // Handle overflow case
    if (current_time < previous_time) {
        // Overflow occurred - calculate difference across the overflow boundary
        return (UINT32_MAX - previous_time) + current_time + 1;
    }

    return current_time - previous_time;
}

}  // namespace jenlib::time
#endif

