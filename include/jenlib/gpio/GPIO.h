//! @file jenlib/gpio/GPIO.h
//! @brief Public wrapper API for GPIO operations.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)


#ifndef INCLUDE_JENLIB_GPIO_GPIO_H_
#define INCLUDE_JENLIB_GPIO_GPIO_H_

#include <jenlib/gpio/GpioDriver.h>
#include <cstdint>

//! @namespace GPIO
//! @brief Public wrapper API for GPIO operations.
namespace GPIO {

//! @brief Pin mode for a GPIO pin (aliased to core driver enum).
using PinMode = jenlib::gpio::PinMode;

//! @brief Logical digital value for GPIO reads/writes (aliased to core driver enum).
using DigitalValue = jenlib::gpio::DigitalValue;

//! @brief Hardware-defined pin index; user creates mapping (aliased to core driver type).
using PinIndex = jenlib::gpio::PinIndex;

//! Lightweight handle to a GPIO pin; forwards calls to the active driver.
class Pin {
 public:
    //! Default constructor creates a pin handle for index 0.
    Pin() noexcept;

    //! Construct a pin handle for a specific pin index.
    explicit Pin(PinIndex pin_index) noexcept;

    //! Set the mode of the pin.
    void pinMode(PinMode mode) const noexcept;
    //! Write a digital value to the pin.
    void digitalWrite(DigitalValue value) const noexcept;
    //! Write an analog value (DAC/PWM) to the pin.
    void analogWrite(std::uint16_t value) const noexcept;
    //! Read a digital value from the pin.
    DigitalValue digitalRead() const noexcept;
    //! Read an analog value (ADC) from the pin.
    std::uint16_t analogRead() const noexcept;

    //! Get the raw pin index for use with libraries that require raw pin numbers.
    //! @return The underlying pin index as used by the driver.
    PinIndex getIndex() const noexcept { return raw_pin_; }

    //! Implicit conversion to raw pin number for compatibility with existing libraries.
    //! @return The underlying pin index as a raw integer.
    operator PinIndex() const noexcept { return raw_pin_; }

    ~Pin() = default;

 private:
    // Raw pin number passed to the driver.
    std::uint8_t raw_pin_;
};

//! Configure analog conversion and PWM resolution for the platform.
void setAnalogReadResolution(std::uint8_t bits) noexcept;
void setAnalogWriteResolution(std::uint8_t bits) noexcept;
std::uint8_t getAnalogReadResolution() noexcept;
std::uint8_t getAnalogWriteResolution() noexcept;

//! Driver injection for testability and portability.
void setDriver(jenlib::gpio::GpioDriver* driver) noexcept;
jenlib::gpio::GpioDriver* getDriver() noexcept;

//! Singleton accessor providing `operator[]` to construct pins by index.
class PinMap {
 public:
    PinMap();
    ~PinMap();
    //! Access the GPIO pin with the given index.
    Pin operator[](PinIndex index) const noexcept;
};

}  // namespace GPIO

#endif  // INCLUDE_JENLIB_GPIO_GPIO_H_



