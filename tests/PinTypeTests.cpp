//! @file tests/PinTypeTests.cpp
//! @brief Tests for type-safe pin wrappers.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <unity.h>
#include <cstdint>
#include "jenlib/gpio/GpioDriver.h"
#include "jenlib/gpio/drivers/NativeGpioDriver.h"
#include "jenlib/gpio/GPIO.h"
#include "jenlib/gpio/PinTypes.h"
#include "jenlib/onewire/OneWireBus.h"
// Use fully-qualified names instead of using-directives

static jenlib::gpio::NativeGpioDriver driver;

//! @test test_typed_pin_construction
//! @brief Verifies typed pin construction from GPIO::Pin and raw index.
//! @details Tests that TypedPin can be constructed from both GPIO::Pin and raw indices.
void test_typed_pin_construction(void) {
    GPIO::setDriver(&driver);

    // Test construction from GPIO::Pin
    GPIO::Pin base_pin(13);
    GPIO::OneWirePin onewire_pin(base_pin);
    TEST_ASSERT_EQUAL_UINT8(13, onewire_pin.getIndex());

    // Test construction from raw index
    GPIO::OneWirePin onewire_pin2(14);
    TEST_ASSERT_EQUAL_UINT8(14, onewire_pin2.getIndex());

    // Test factory function
    auto onewire_pin3 = GPIO::makeTypedPin<GPIO::PinTags::OneWire>(15);
    TEST_ASSERT_EQUAL_UINT8(15, onewire_pin3.getIndex());
}

//! @test test_typed_pin_conversion
//! @brief Verifies typed pin conversion to raw pin numbers and GPIO::Pin.
//! @details Tests implicit conversion operators for library compatibility.
void test_typed_pin_conversion(void) {
    GPIO::setDriver(&driver);

    GPIO::OneWirePin onewire_pin(16);

    // Test implicit conversion to raw pin number
    std::uint8_t raw_pin = onewire_pin;
    TEST_ASSERT_EQUAL_UINT8(16, raw_pin);

    // Test implicit conversion to GPIO::Pin
    GPIO::Pin base_pin = onewire_pin;
    TEST_ASSERT_EQUAL_UINT8(16, base_pin.getIndex());

    // Test arrow operator access
    onewire_pin->pinMode(GPIO::PinMode::OUTPUT);
    onewire_pin->digitalWrite(GPIO::DigitalValue::HIGH);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(jenlib::gpio::DigitalValue::HIGH),
                            static_cast<uint8_t>(driver.digital_read(16)));
}

//! @test test_onewire_type_safety
//! @brief Verifies OneWire bus construction with type-safe pins.
//! @details Tests that OneWire can be constructed with typed pins and maintains type safety.
void test_onewire_type_safety(void) {
    GPIO::setDriver(&driver);

    // Test construction with typed pin
    GPIO::OneWirePin onewire_pin(17);
    OneWire::OneWireBus bus1(onewire_pin);
    bus1.begin();
    TEST_ASSERT_TRUE(bus1.reset());  // Should work with our mock implementation

    // Test construction with GPIO::Pin
    GPIO::Pin base_pin(18);
    OneWire::OneWireBus bus2(base_pin);
    bus2.begin();
    TEST_ASSERT_TRUE(bus2.reset());

    // Test construction with raw pin (backward compatibility)
    OneWire::OneWireBus bus3(19);
    bus3.begin();
    TEST_ASSERT_TRUE(bus3.reset());

    // Test factory function for OneWire pins
    auto onewire_pin4 = GPIO::makeTypedPin<GPIO::PinTags::OneWire>(20);
    OneWire::OneWireBus bus4(onewire_pin4);
    bus4.begin();
    TEST_ASSERT_TRUE(bus4.reset());
}

//! @test test_pin_operations_through_typed_pin
//! @brief Verifies GPIO operations work through typed pin wrappers.
//! @details Tests that all GPIO operations are accessible through the typed pin interface.
void test_pin_operations_through_typed_pin(void) {
    GPIO::setDriver(&driver);

    GPIO::OneWirePin onewire_pin(20);

    // Test pin mode setting
    onewire_pin->pinMode(GPIO::PinMode::OUTPUT);

    // Test digital write/read
    onewire_pin->digitalWrite(GPIO::DigitalValue::HIGH);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(jenlib::gpio::DigitalValue::HIGH),
                            static_cast<uint8_t>(driver.digital_read(20)));

    // Test analog operations
    onewire_pin->analogWrite(1024);
    TEST_ASSERT_EQUAL_UINT16(1024, onewire_pin->analogRead());
}

//! @test test_different_pin_types
//! @brief Verifies different pin types can be created and used independently.
//! @details Tests that different typed pins (SPI, I2C, etc.) work correctly.
void test_different_pin_types(void) {
    GPIO::setDriver(&driver);

    // Test different pin types
    GPIO::SPIPin spi_pin(21);
    GPIO::I2CPin i2c_pin(22);
    GPIO::PWMPin pwm_pin(23);
    GPIO::ADCPin adc_pin(24);
    GPIO::DigitalPin digital_pin(25);

    // All should have correct indices
    TEST_ASSERT_EQUAL_UINT8(21, spi_pin.getIndex());
    TEST_ASSERT_EQUAL_UINT8(22, i2c_pin.getIndex());
    TEST_ASSERT_EQUAL_UINT8(23, pwm_pin.getIndex());
    TEST_ASSERT_EQUAL_UINT8(24, adc_pin.getIndex());
    TEST_ASSERT_EQUAL_UINT8(25, digital_pin.getIndex());

    // All should support GPIO operations
    spi_pin->pinMode(GPIO::PinMode::OUTPUT);
    i2c_pin->pinMode(GPIO::PinMode::INPUT_PULLUP);
    pwm_pin->analogWrite(512);
    adc_pin->pinMode(GPIO::PinMode::INPUT);
    digital_pin->digitalWrite(GPIO::DigitalValue::LOW);
    // Validate observable effects
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(jenlib::gpio::DigitalValue::LOW),
                            static_cast<uint8_t>(driver.digital_read(25)));
    TEST_ASSERT_EQUAL_UINT16(512, driver.analog_read(23));
}

//! @test test_onewire_basic_operations
//! @brief Verifies basic OneWire operations work correctly.
//! @details Tests write/read operations and ROM commands.
void test_onewire_basic_operations(void) {
    GPIO::setDriver(&driver);

    GPIO::OneWirePin onewire_pin(21);
    OneWire::OneWireBus bus(onewire_pin);
    bus.begin();

    // Test basic operations
    TEST_ASSERT_TRUE(bus.reset());

    // Test write/read operations
    bus.write_byte(0x55);
    OneWire::OneWireBus::byte read_data = bus.read_byte();
    // Note: In our mock implementation, read_byte returns 0, which is expected

    // Test ROM operations
    bus.skip_rom();

    // Test ROM code operations
    OneWire::OneWireBus::rom_code_t rom_code = {0x28, 0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};
    bus.match_rom(rom_code);

    // Test read ROM (should work in our mock)
    OneWire::OneWireBus::rom_code_t read_rom = {0};
    TEST_ASSERT_TRUE(bus.read_rom(read_rom));
    // In mock, read_byte returns 0
    TEST_ASSERT_EQUAL_UINT8(0, read_data);
}

