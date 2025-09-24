//! @file jenlib/gpio/drivers/ArduinoGpioDriver.h
//! @brief Arduino GPIO driver implementation.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_GPIO_DRIVERS_ARDUINOGPIODRIVER_H_
#define INCLUDE_JENLIB_GPIO_DRIVERS_ARDUINOGPIODRIVER_H_

#include <jenlib/gpio/GpioDriver.h>
#include <cstdint>

namespace jenlib::gpio {

class ArduinoGpioDriver : public GpioDriver {
 public:
    ~ArduinoGpioDriver() override = default;

    void set_pin_mode(PinIndex pin, PinMode mode) noexcept override;
    void digital_write(PinIndex pin, DigitalValue value) noexcept override;
    DigitalValue digital_read(PinIndex pin) noexcept override;

    void analog_write(PinIndex pin, std::uint16_t value) noexcept override;
    std::uint16_t analog_read(PinIndex pin) noexcept override;

    void set_analog_read_resolution(std::uint8_t bits) noexcept override;
    void set_analog_write_resolution(std::uint8_t bits) noexcept override;
    std::uint8_t get_analog_read_resolution() const noexcept override;
    std::uint8_t get_analog_write_resolution() const noexcept override;

 private:
    std::uint8_t analog_read_bits_{10};
    std::uint8_t analog_write_bits_{8};
};

} // namespace jenlib::gpio

#endif  // INCLUDE_JENLIB_GPIO_DRIVERS_ARDUINOGPIODRIVER_H_



