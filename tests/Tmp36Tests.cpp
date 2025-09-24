//! @file tests/Tmp36Tests.cpp
//! @brief TMP36 tests.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <cstdint>
#include "unity.h"


// Local conversion function (example-style) to avoid embedding sensor logic in the lib
static float tmp36_celsius_from_code_local(std::uint16_t code, std::uint8_t bits, float vref_volts) {
    if (bits == 0 || vref_volts <= 0.0f) return 0.0f;
    const float max_code = static_cast<float>((1u << bits) - 1u);
    const float volts = (static_cast<float>(code) / max_code) * vref_volts;
    return (volts - 0.5f) * 100.0f;
}

//! @test test_tmp36_conversion_10bit_5v
//! @brief Verifies TMP36 temperature conversion with 10-bit ADC and 5V reference.
//! @details Tests ADC code 153 (representing 0.75V) converts to ~25°C.
void test_tmp36_conversion_10bit_5v(void) {
    // 25 C -> 0.75 V -> code ~ 153 (10-bit, 5V)
    const float vref = 5.0f;
    const uint8_t bits = 10;
    const uint16_t code = 153;
    float c = tmp36_celsius_from_code_local(code, bits, static_cast<float>(vref));
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 25.0f, c);
}

//! @test test_tmp36_conversion_12bit_3v3
//! @brief Verifies TMP36 temperature conversion with 12-bit ADC and 3.3V reference.
//! @details Tests ADC code 993 (representing 0.8V) converts to ~30°C.
void test_tmp36_conversion_12bit_3v3(void) {
    // 30 C -> 0.8 V -> code ~ (0.8/3.3)*4095 ≈ 993
    const float vref = 3.3f;
    const uint8_t bits = 12;
    const uint16_t code = 993;
    float c = tmp36_celsius_from_code_local(code, bits, static_cast<float>(vref));
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 30.0f, c);
}



