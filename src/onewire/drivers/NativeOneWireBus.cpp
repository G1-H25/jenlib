//! @file src/onewire/drivers/NativeOneWireBus.cpp
//! @brief Native OneWire bus implementation for testing and development.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#if !defined(ARDUINO) && !defined(ESP_PLATFORM)

#include <jenlib/onewire/OneWireBus.h>
#include <jenlib/gpio/GPIO.h>
#include <cstdint>

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
    configure_pin();
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
    return perform_reset();
}

//! @brief Write a single byte (LSB first on the wire).
void OneWireBus::write_byte(byte data) {
    if (!initialized_) {
        return;
    }

    for (int i = 0; i < 8; ++i) {
        write_bit((data & 0x01) != 0);
        data >>= 1;
    }
}

//! @brief Read a single byte (LSB first on the wire).
OneWireBus::byte OneWireBus::read_byte() {
    if (!initialized_) {
        return 0;
    }

    byte data = 0;
    for (int i = 0; i < 8; ++i) {
        data >>= 1;
        if (read_bit()) {
            data |= 0x80;
        }
    }
    return data;
}

//! @brief Send SKIP ROM (address all devices).
void OneWireBus::skip_rom() {
    if (!initialized_) {
        return;
    }
    write_byte(static_cast<byte>(Command::SkipRom));
}

//! @brief Send MATCH ROM with the provided device address.
void OneWireBus::match_rom(const rom_code_t& rom) {
    if (!initialized_) {
        return;
    }

    write_byte(static_cast<byte>(Command::MatchRom));
    for (const auto& byte_val : rom) {
        write_byte(byte_val);
    }
}

//! @brief Read the ROM code (only valid on single-drop bus).
bool OneWireBus::read_rom(rom_code_t& out_rom) {
    if (!initialized_) {
        return false;
    }

    if (!perform_reset()) {
        return false;
    }

    write_byte(static_cast<byte>(Command::ReadRom));
    for (auto& byte_val : out_rom) {
        byte_val = read_byte();
    }
    return true;
}

//! @brief Internal method to configure the pin for OneWire use.
void OneWireBus::configure_pin() {
    // Configure the pin as output for OneWire communication
    GPIO::Pin gpio_pin(pin_);
    gpio_pin.pinMode(GPIO::PinMode::OUTPUT);
    gpio_pin.digitalWrite(GPIO::DigitalValue::HIGH);
}

//! @brief Internal method to perform a reset pulse.
bool OneWireBus::perform_reset() {
    GPIO::Pin gpio_pin(pin_);

    // Pull line low for 480us (reset pulse)
    gpio_pin.digitalWrite(GPIO::DigitalValue::LOW);
    // Note: In a real implementation, you'd need proper timing here
    // For now, we'll simulate the timing

    // Release the line and check for presence pulse
    gpio_pin.pinMode(GPIO::PinMode::INPUT_PULLUP);
    // Check if any device pulls the line low (presence pulse)
    // This is a simplified implementation for testing

    // Restore output mode
    gpio_pin.pinMode(GPIO::PinMode::OUTPUT);
    gpio_pin.digitalWrite(GPIO::DigitalValue::HIGH);

    // For testing purposes, always return true (device present)
    return true;
}

//! @brief Internal method to write a single bit.
void OneWireBus::write_bit(bool bit) {
    GPIO::Pin gpio_pin(pin_);

    if (bit) {
        // Write 1: Pull low for 6us, then release
        gpio_pin.digitalWrite(GPIO::DigitalValue::LOW);
        // Note: In real implementation, wait 6us
        gpio_pin.digitalWrite(GPIO::DigitalValue::HIGH);
        // Note: In real implementation, wait 64us total
    } else {
        // Write 0: Pull low for 60us, then release
        gpio_pin.digitalWrite(GPIO::DigitalValue::LOW);
        // Note: In real implementation, wait 60us
        gpio_pin.digitalWrite(GPIO::DigitalValue::HIGH);
        // Note: In real implementation, wait 10us total
    }
}

//! @brief Internal method to read a single bit.
bool OneWireBus::read_bit() {
    GPIO::Pin gpio_pin(pin_);

    // Pull line low for 6us
    gpio_pin.digitalWrite(GPIO::DigitalValue::LOW);
    // Note: In real implementation, wait 6us

    // Release line and sample after 9us
    gpio_pin.pinMode(GPIO::PinMode::INPUT_PULLUP);
    // Note: In real implementation, wait 9us then sample
    bool bit_value = (gpio_pin.digitalRead() == GPIO::DigitalValue::HIGH);

    // Restore output mode
    gpio_pin.pinMode(GPIO::PinMode::OUTPUT);
    gpio_pin.digitalWrite(GPIO::DigitalValue::HIGH);

    return bit_value;
}

}  // namespace OneWire

#endif  // !ARDUINO && !ESP_PLATFORM
