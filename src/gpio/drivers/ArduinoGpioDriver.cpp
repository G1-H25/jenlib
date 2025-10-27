//! @file src/gpio/drivers/ArduinoGpioDriver.cpp
//! @brief Arduino GPIO driver implementation.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)


#include <jenlib/gpio/drivers/ArduinoGpioDriver.h>

#ifdef ARDUINO
#include <Arduino.h>

//! @namespace jenlib::gpio
//! @brief GPIO namespace.
namespace jenlib::gpio {

//! @brief Set the pin mode. Uses Arduino API.
void ArduinoGpioDriver::set_pin_mode(PinIndex pin, PinMode mode) noexcept {
    uint8_t m = INPUT;
    //! @brief Set the pin mode.
    //! @param pin The pin index.
    //! @param mode The pin mode.
    switch (mode) {
        case PinMode::INPUT: m = INPUT; break;
        case PinMode::OUTPUT: m = OUTPUT; break;
        case PinMode::INPUT_PULLUP: m = INPUT_PULLUP; break;
        case PinMode::INPUT_PULLDOWN:
#ifdef INPUT_PULLDOWN
            m = INPUT_PULLDOWN;
#else
            m = INPUT;
#endif
            break;
    }
    pinMode(pin, m);
}

//! @brief Write a digital value to a pin. Uses Arduino API.
void ArduinoGpioDriver::digital_write(PinIndex pin, DigitalValue value) noexcept {
    digitalWrite(pin, value == DigitalValue::HIGH ? HIGH : LOW);
}

//! @brief Read a digital value from a pin. Uses Arduino API.
DigitalValue ArduinoGpioDriver::digital_read(PinIndex pin) noexcept {
    int v = digitalRead(pin);
    return v == HIGH ? DigitalValue::HIGH : DigitalValue::LOW;
}

//! @brief Write an analog value to a pin. Uses Arduino API.
void ArduinoGpioDriver::analog_write(PinIndex pin, std::uint16_t value) noexcept {
#ifdef analogWriteResolution
    analogWrite(pin, value);
#else
    uint8_t out = (analog_write_bits_ <= 8)
        ? static_cast<uint8_t>(value)
        : static_cast<uint8_t>(value >> (analog_write_bits_ - 8));
    analogWrite(pin, out);
#endif
}

//! @brief Read an analog value from a pin. Uses Arduino API.
std::uint16_t ArduinoGpioDriver::analog_read(PinIndex pin) noexcept {
    return static_cast<std::uint16_t>(analogRead(pin));
}

//! @brief Set the analog read resolution. Uses Arduino API.
void ArduinoGpioDriver::set_analog_read_resolution(std::uint8_t bits) noexcept {
    analog_read_bits_ = bits;
#ifdef analogReadResolution
    analogReadResolution(bits);
#endif
}

//! @brief Set the analog write resolution. Uses Arduino API.
void ArduinoGpioDriver::set_analog_write_resolution(std::uint8_t bits) noexcept {
    analog_write_bits_ = bits;
#ifdef analogWriteResolution
    analogWriteResolution(bits);
#endif
}

//! @brief Get the analog read resolution. Uses Arduino API.
std::uint8_t ArduinoGpioDriver::get_analog_read_resolution() const noexcept {
    return analog_read_bits_;
}

//! @brief Get the analog write resolution. Uses Arduino API.
std::uint8_t ArduinoGpioDriver::get_analog_write_resolution() const noexcept {
    return analog_write_bits_;
}

}  // namespace jenlib::gpio

#else
// Empty implementation for non-Arduino platforms
// The header file already provides deleted constructors
namespace jenlib::gpio {
    // No implementation needed - constructors are deleted in header
}

#endif  // ARDUINO



