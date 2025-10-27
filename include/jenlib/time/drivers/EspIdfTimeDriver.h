//! @file include/jenlib/time/drivers/EspIdfTimeDriver.h
//! @brief ESP-IDF time driver interface.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_TIME_DRIVERS_ESPIDF_TIMEDRIVER_H_
#define INCLUDE_JENLIB_TIME_DRIVERS_ESPIDF_TIMEDRIVER_H_

#include <jenlib/time/TimeDriver.h>

#ifdef ESP_PLATFORM
#include <esp_timer.h>

namespace jenlib::time {

//! @brief ESP-IDF time driver implementation using esp_timer.
class EspIdfTimeDriver : public TimeDriver {
 public:
    //! @brief Constructor.
    EspIdfTimeDriver() = default;
    
    //! @brief Destructor.
    ~EspIdfTimeDriver() = default;

    //! @brief Get current time in milliseconds using esp_timer.
    std::uint32_t now() noexcept override;
    
    //! @brief Delay execution for specified milliseconds using esp_timer.
    void delay(std::uint32_t delay_ms) noexcept override;
    
    //! @brief Check if time value has overflowed.
    bool has_overflowed(std::uint32_t time_value) noexcept override;
    
    //! @brief Calculate time difference handling overflow.
    std::uint32_t time_difference(std::uint32_t current_time, 
                                  std::uint32_t previous_time) noexcept override;

 private:
    std::uint32_t last_time_ = 0;      //!< Last recorded time for overflow detection
    std::uint32_t overflow_count_ = 0; //!< Overflow counter
};

}  // namespace jenlib::time

#else
// Fallback implementation for non-ESP platforms
namespace jenlib::time {

//! @brief Empty stub implementation for non-ESP platforms.
class EspIdfTimeDriver {
 public:
    EspIdfTimeDriver() = delete;
};

}  // namespace jenlib::time

#endif  // ESP_PLATFORM

#endif  // INCLUDE_JENLIB_TIME_DRIVERS_ESPIDF_TIMEDRIVER_H_
