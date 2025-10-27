
//! @file src/onewire/drivers/ArduinoOneWireBus.cpp
//! @brief Arduino OneWire bus wrapper implementation
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifdef JENLIB_ENABLE_ARDUINO_ONEWIRE

#include <jenlib/onewire/OneWireBus.h>
#include <OneWire.h>

namespace OneWire {

//! @brief Constructor with type-safe pin.
OneWireBus::OneWireBus(GPIO::OneWirePin pin)
    : pin_(static_cast<std::uint8_t>(pin.getIndex())), initialized_(false) {}

//! @brief Constructor with generic GPIO pin.
OneWireBus::OneWireBus(GPIO::Pin pin)
    : pin_(static_cast<std::uint8_t>(pin.getIndex())), initialized_(false) {}

//! @brief Constructor with raw pin number.
OneWireBus::OneWireBus(std::uint8_t raw_pin)
    : pin_(raw_pin), initialized_(false) {}

//! @brief Initialize the bus and configure the GPIO.
void OneWireBus::begin() {
    // OneWire library handles pin configuration internally
    initialized_ = true;
}

//! @brief Release any resources associated with the bus.
void OneWireBus::end() {
    initialized_ = false;
}

//! @brief Issue reset pulse and detect presence.
bool OneWireBus::reset() {
    if (!initialized_) {
        return false;
    }
    // Use Arduino OneWire library for actual implementation
    OneWire ow(pin_);
    return ow.reset() == 1;
}

//! @brief Write a single byte (LSB first on the wire).
void OneWireBus::write_byte(byte data) {
    if (!initialized_) {
        return;
    }
    OneWire ow(pin_);
    ow.write(data);
}

//! @brief Read a single byte (LSB first on the wire).
OneWireBus::byte OneWireBus::read_byte() {
    if (!initialized_) {
        return 0;
    }
    OneWire ow(pin_);
    return static_cast<byte>(ow.read());
}

//! @brief Send SKIP ROM (address all devices).
void OneWireBus::skip_rom() {
    if (!initialized_) {
        return;
    }
    OneWire ow(pin_);
    ow.skip();
}

//! @brief Send MATCH ROM with the provided device address.
void OneWireBus::match_rom(const rom_code_t& rom) {
    if (!initialized_) {
        return;
    }
    OneWire ow(pin_);
    ow.select(const_cast<byte*>(rom.data()));
}

//! @brief Read the ROM code (only valid on single-drop bus).
bool OneWireBus::read_rom(rom_code_t& out_rom) {
    if (!initialized_) {
        return false;
    }
    OneWire ow(pin_);
    ow.reset();
    ow.write(static_cast<byte>(Command::ReadRom));
    for (int i = 0; i < 8; ++i) {
        out_rom[static_cast<std::size_t>(i)] = static_cast<byte>(ow.read());
    }
    return true;
}

//! @brief Internal method to configure the pin for OneWire use.
void OneWireBus::configure_pin() {
    // Arduino OneWire library handles pin configuration
}

//! @brief Internal method to perform a reset pulse.
bool OneWireBus::perform_reset() {
    OneWire ow(pin_);
    return ow.reset() == 1;
}

//! @brief Internal method to write a single bit.
void OneWireBus::write_bit(bool bit) {
    OneWire ow(pin_);
    ow.write_bit(bit ? 1 : 0);
}

//! @brief Internal method to read a single bit.
bool OneWireBus::read_bit() {
    OneWire ow(pin_);
    return ow.read_bit() == 1;
}

}  // namespace OneWire

#else
// Empty implementation for non-Arduino platforms
// This implementation is only available when JENLIB_ENABLE_ARDUINO_ONEWIRE is defined
namespace OneWire {
    // No implementation needed - this file is only compiled when JENLIB_ENABLE_ARDUINO_ONEWIRE is defined
}

#endif  // JENLIB_ENABLE_ARDUINO_ONEWIRE
