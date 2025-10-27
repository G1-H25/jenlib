//! @file src/time/drivers/EspIdfTimeDriver.cpp
//! @brief ESP-IDF time driver implementation using esp_timer.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <jenlib/time/drivers/EspIdfTimeDriver.h>

#ifdef ESP_PLATFORM
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace jenlib::time {

std::uint32_t EspIdfTimeDriver::now() noexcept {
    // Get time in microseconds and convert to milliseconds
    std::uint64_t time_us = esp_timer_get_time();
    std::uint32_t current_time = static_cast<std::uint32_t>(time_us / 1000);

    // Check for overflow (esp_timer_get_time() wraps around after ~49.7 days)
    if (current_time < last_time_) {
        ++overflow_count_;
    }
    last_time_ = current_time;

    return current_time;
}

void EspIdfTimeDriver::delay(std::uint32_t delay_ms) noexcept {
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
}

bool EspIdfTimeDriver::has_overflowed(std::uint32_t time_value) noexcept {
    return time_value < last_time_;
}

std::uint32_t EspIdfTimeDriver::time_difference(std::uint32_t current_time, 
                                                 std::uint32_t previous_time) noexcept {
    // Handle overflow case
    if (current_time < previous_time) {
        // Overflow occurred - calculate difference across the overflow boundary
        return (UINT32_MAX - previous_time) + current_time + 1;
    }

    return current_time - previous_time;
}

}  // namespace jenlib::time

#else
// Empty implementation for non-ESP platforms
// The header file already provides deleted constructors
namespace jenlib::time {
    // No implementation needed - constructors are deleted in header
}

#endif  // ESP_PLATFORM
