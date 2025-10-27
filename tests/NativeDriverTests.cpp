
//! @file tests/NativeDriverTests.cpp
//! @brief Native driver tests.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <unity.h>
#include <cstdint>
#include "jenlib/gpio/GPIO.h"
#include "jenlib/gpio/drivers/NativeGpioDriver.h"


static jenlib::gpio::NativeGpioDriver native_driver;

//! @test test_native_driver_roundtrip
//! @brief Verifies basic digital operations with NativeGpioDriver.
//! @details Sets pin mode, writes HIGH, then verifies driver stored the value.
void test_native_driver_roundtrip(void) {
    GPIO::setDriver(&native_driver);
    GPIO::Pin p(5);
    p.pinMode(GPIO::PinMode::OUTPUT);
    p.digitalWrite(GPIO::DigitalValue::HIGH);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(jenlib::gpio::DigitalValue::HIGH),
                            static_cast<uint8_t>(native_driver.digital_read(5)));
}

//! @test test_native_driver_analog
//! @brief Verifies analog write/read operations with NativeGpioDriver.
//! @details Writes analog value and reads it back to verify storage.
void test_native_driver_analog(void) {
    GPIO::setDriver(&native_driver);
    GPIO::Pin p(6);
    p.analogWrite(1234);
    TEST_ASSERT_EQUAL_UINT16(1234, p.analogRead());
}

static float tmp36_voltage_from_celsius(float celsius) {
    // TMP36: 750 mV at 25 C, 10 mV/°C slope; 0 C => 500 mV
    return 0.5 + (celsius * 0.01);
}

//! @test test_voltage_levels_and_tmp36
//! @brief Demonstrates voltage modeling and sensor simulation.
//! @details Sets up TMP36 voltage model, tests ADC conversion at different resolutions,
//! and verifies digital threshold behavior based on voltage levels.
void test_voltage_levels_and_tmp36(void) {
    GPIO::setDriver(&native_driver);
    native_driver.set_reference_voltage(5.0);
    native_driver.set_digital_threshold_ratio(0.5);  // 2.5 V threshold
    // Simulate TMP36 at 25 C -> 0.75 V
    native_driver.set_pin_voltage(7, tmp36_voltage_from_celsius(25.0));
    GPIO::Pin sensor(7);

    // With 10-bit ADC
    GPIO::setAnalogReadResolution(10);
    std::uint16_t code10 = sensor.analogRead();
    // Expected ~ (0.75/5.0) * 1023 ≈ 153
    TEST_ASSERT_INT_WITHIN(2, 153, code10);

    // With 12-bit ADC
    GPIO::setAnalogReadResolution(12);
    std::uint16_t code12 = sensor.analogRead();
    // Expected ~ (0.75/5.0) * 4095 ≈ 614
    TEST_ASSERT_INT_WITHIN(4, 614, code12);

    // Digital threshold behavior
    // 0.75 V < 2.5 V => LOW
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(jenlib::gpio::DigitalValue::LOW),
                            static_cast<uint8_t>(sensor.digitalRead()));

    // Raise to 3.3 V => HIGH
    native_driver.set_pin_voltage(7, 3.3);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(jenlib::gpio::DigitalValue::HIGH),
                            static_cast<uint8_t>(sensor.digitalRead()));
}
