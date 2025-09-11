//! @file jenlib/gpio/GPIO.h
//! @brief Public wrapper API for GPIO operations.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)


#ifndef INCLUDE_JENLIB_GPIO_GPIO_H_
#define INCLUDE_JENLIB_GPIO_GPIO_H_

#include <jenlib/gpio/GpioDriver.h>
#include <cstdint>

//! @namespace GPIO
//! @brief Public wrapper API for GPIO operations.
namespace GPIO {

//! @enum PinMode
//! @brief Pin mode for a GPIO pin.
enum class PinMode : std::uint8_t {
INPUT,
OUTPUT,
INPUT_PULLUP,
INPUT_PULLDOWN,
};

//! @enum DigitalValue
//! @brief Logical digital value for GPIO reads/writes.
enum class DigitalValue : std::uint8_t {
LOW,
HIGH,
};

//! @typedef PinIndex
//! @brief Hardware-defined pin index; user creates mapping.
//! @details Create a mapping between the pin index and the GPIO pin number.
//! For instance, the mapping for Arduino Uno R4 Wifi would be:
//! ```
//! {
//!     class enum PinIndex {
//!         D0 = 0, //!< UART TX
//!         D1 = 1, //!< UART RX
//!         D2 = 2,
//!         D3 = 3, 
//!         D4 = 4,
//!         D5 = 5,
//!         D6 = 6,
//!         D7 = 7,
//!         D8 = 8,
//!         D9 = 9, //!< SPI SS
//!         D10 = 10, //!< SPI MOSI
//!         D11 = 11, //!< SPI MISO
//!         D12 = 12, //!< SPI SCK
//!         D13 = 13, //!< Built-in LED, do not use for digital input
//!         A0 = 14, //!< ADC
//!         A1 = 15,
//!         A2 = 16,
//!         A3 = 17,
//!         A4 = 18,
//!         A5 = 19,
//!         A6 = 20,
//!     };
//! }
//! ```
using PinIndex = std::uint8_t;

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
void setDriver(::gpio::GpioDriver* driver) noexcept;
::gpio::GpioDriver* getDriver() noexcept;

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


