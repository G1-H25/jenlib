//! @file include/jenlib/gpio/drivers/EspIdfGpioDriver.h
//! @brief ESP-IDF GPIO driver interface.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_GPIO_DRIVERS_ESPIDFGPIODRIVER_H_
#define INCLUDE_JENLIB_GPIO_DRIVERS_ESPIDFGPIODRIVER_H_

#include <jenlib/gpio/GpioDriver.h>
#include <array>
#include <unordered_map>

#ifdef ESP_PLATFORM
#include <driver/gpio.h>
#include <driver/adc.h>
#include <driver/ledc.h>

namespace jenlib::gpio {

//! @brief ESP-IDF GPIO driver implementation.
class EspIdfGpioDriver : public GpioDriver {
 public:
    //! @brief Constructor.
    EspIdfGpioDriver() = default;

    //! @brief Destructor.
    ~EspIdfGpioDriver() = default;

    //! @brief Set the pin mode using ESP-IDF GPIO API.
    void set_pin_mode(PinIndex pin, PinMode mode) noexcept override;

    //! @brief Write a digital value to a pin using ESP-IDF GPIO API.
    void digital_write(PinIndex pin, DigitalValue value) noexcept override;

    //! @brief Read a digital value from a pin using ESP-IDF GPIO API.
    DigitalValue digital_read(PinIndex pin) noexcept override;

    //! @brief Write an analog value to a pin using ESP-IDF LEDC API.
    void analog_write(PinIndex pin, std::uint16_t value) noexcept override;

    //! @brief Read an analog value from a pin using ESP-IDF ADC API.
    std::uint16_t analog_read(PinIndex pin) noexcept override;

    //! @brief Set the analog read resolution.
    void set_analog_read_resolution(std::uint8_t bits) noexcept override;

    //! @brief Set the analog write resolution.
    void set_analog_write_resolution(std::uint8_t bits) noexcept override;

    //! @brief Get the analog read resolution.
    std::uint8_t get_analog_read_resolution() const noexcept override;

    //! @brief Get the analog write resolution.
    std::uint8_t get_analog_write_resolution() const noexcept override;

 private:
    std::uint8_t analog_read_bits_ = 12;   //!< ADC resolution in bits
    std::uint8_t analog_write_bits_ = 8;  //!< PWM resolution in bits

    // LEDC lazy-initialization state
    bool ledc_initialized_ = false;

    // Channel allocation: map pin -> channel and track used channels
    std::unordered_map<int, ledc_channel_t> pin_to_channel_;
    std::array<bool, LEDC_CHANNEL_MAX> channel_used_{};

    //! @brief Get or allocate an LEDC channel for a GPIO pin.
    ledc_channel_t get_or_allocate_channel_for_pin(int gpio_pin) noexcept;

    //! @brief Convert jenlib PinMode to ESP-IDF gpio_mode_t.
    gpio_mode_t convert_pin_mode(PinMode mode) const noexcept;

    //! @brief Convert jenlib PinMode to ESP-IDF gpio_pull_mode_t.
    gpio_pull_mode_t convert_pull_mode(PinMode mode) const noexcept;

    //! @brief Convert bit width to ESP-IDF adc_bits_width_t enum.
    adc_bits_width_t convert_adc_bits_width(std::uint8_t bits) const noexcept;
};

}  // namespace jenlib::gpio

#else
// Fallback implementation for non-ESP platforms
namespace jenlib::gpio {

//! @brief Empty stub implementation for non-ESP platforms.
class EspIdfGpioDriver {
 public:
    EspIdfGpioDriver() = delete;
};

}  // namespace jenlib::gpio

#endif  // ESP_PLATFORM

#endif  // INCLUDE_JENLIB_GPIO_DRIVERS_ESPIDFGPIODRIVER_H_
