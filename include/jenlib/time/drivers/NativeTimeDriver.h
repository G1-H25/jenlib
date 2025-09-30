//! @file include/jenlib/time/drivers/NativeTimeDriver.h
//! @brief Native (desktop) time driver implementation
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_TIME_DRIVERS_NATIVETIMEDRIVER_H_
#define INCLUDE_JENLIB_TIME_DRIVERS_NATIVETIMEDRIVER_H_

#include <chrono>
#include "jenlib/time/TimeDriver.h"
#include "jenlib/time/TimeTypes.h"

namespace jenlib::time {

//! @brief Native (desktop) time driver
//! @details
//! Provides time functionality using std::chrono for native desktop environments.
//! Uses steady_clock for consistent timing across different platforms.
class NativeTimeDriver : public TimeDriver {
 public:
    //! @brief Get current time in milliseconds using std::chrono
    //! @return Current time in milliseconds
    std::uint32_t now() override;

    //! @brief Delay execution for specified milliseconds using std::this_thread::sleep_for
    //! @param delay_ms Delay duration in milliseconds
    void delay(std::uint32_t delay_ms) override;

    //! @brief Static versions for backward compatibility
    static std::uint32_t now_static();
    static void delay_static(std::uint32_t delay_ms);

    //! @brief Initialize the native time driver
    static void initialize();

    //! @brief Get the time since epoch in milliseconds
    //! @return Time since epoch in milliseconds
    static std::uint64_t get_epoch_time_ms();

 private:
    //! @brief Time point when the driver was initialized
    static std::chrono::steady_clock::time_point start_time_;

    //! @brief Whether the driver has been initialized
    static bool initialized_;
};

}  // namespace jenlib::time

#endif  // INCLUDE_JENLIB_TIME_DRIVERS_NATIVETIMEDRIVER_H_

