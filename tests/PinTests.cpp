//! @file tests/PinTests.cpp
//! @brief Pin tests.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <unity.h>
#include <cstdint>
#include "jenlib/gpio/GpioDriver.h"
#include "jenlib/gpio/drivers/NativeGpioDriver.h"
#include "jenlib/gpio/GPIO.h"
// Use fully-qualified names instead of using-directives

static jenlib::gpio::NativeGpioDriver driver;

//! @test test_digital_roundtrip
//! @brief Verifies digital write/read roundtrip through GPIO wrapper.
//! @details Sets pin mode, writes HIGH, then verifies driver received the value.
void test_digital_roundtrip(void) {
    GPIO::setDriver(&driver);
    GPIO::Pin led(13);
    led.pinMode(GPIO::PinMode::OUTPUT);
    led.digitalWrite(GPIO::DigitalValue::HIGH);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(jenlib::gpio::DigitalValue::HIGH),
                            static_cast<uint8_t>(driver.digital_read(13)));
}

//! @test test_analog_roundtrip
//! @brief Verifies analog write/read roundtrip through GPIO wrapper.
//! @details Sets 12-bit resolution, writes value, then reads back.
void test_analog_roundtrip(void) {
    GPIO::setDriver(&driver);
    driver.set_analog_write_resolution(12);
    GPIO::Pin sensor(2);
    sensor.analogWrite(2048);
    TEST_ASSERT_EQUAL_UINT16(2048, sensor.analogRead());
}

//! @test test_resolution_forwarding
//! @brief Verifies GPIO wrapper forwards resolution settings to driver.
//! @details Sets read/write resolutions and confirms driver received them.
void test_resolution_forwarding(void) {
    GPIO::setDriver(&driver);
    GPIO::setAnalogReadResolution(11);
    GPIO::setAnalogWriteResolution(9);
    TEST_ASSERT_EQUAL_UINT8(11, driver.get_analog_read_resolution());
    TEST_ASSERT_EQUAL_UINT8(9, driver.get_analog_write_resolution());
}



