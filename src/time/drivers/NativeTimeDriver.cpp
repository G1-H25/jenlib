//! @file src/time/drivers/NativeTimeDriver.cpp
//! @brief Native (desktop) time driver implementation
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef ARDUINO

#include "jenlib/time/drivers/NativeTimeDriver.h"
#include <thread>
#include <chrono>


//! @namespace jenlib::time
namespace jenlib::time {

// Static member definitions
std::chrono::steady_clock::time_point NativeTimeDriver::start_time_;
bool NativeTimeDriver::initialized_ = false;

std::uint32_t NativeTimeDriver::now() {
    return now_static();
}

void NativeTimeDriver::delay(std::uint32_t delay_ms) {
    delay_static(delay_ms);
}

std::uint32_t NativeTimeDriver::now_static() {
    if (!initialized_) {
        initialize();
    }

    auto current_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time_);

    return static_cast<std::uint32_t>(duration.count());
}

void NativeTimeDriver::delay_static(std::uint32_t delay_ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
}

void NativeTimeDriver::initialize() {
    if (!initialized_) {
        start_time_ = std::chrono::steady_clock::now();
        initialized_ = true;
    }
}

std::uint64_t NativeTimeDriver::get_epoch_time_ms() {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return static_cast<std::uint64_t>(duration.count());
}

}  // namespace jenlib::time

#endif  // ARDUINO
