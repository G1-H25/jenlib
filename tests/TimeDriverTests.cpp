//! @file tests/TimeDriverTests.cpp
//! @brief Tests for Time service driver injection
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include "jenlib/time/Time.h"
#include "jenlib/time/drivers/NativeTimeDriver.h"
#include <unity.h>

using namespace jenlib::time;

//! @test test_driver_injection_native
//! @brief Verifies that NativeTimeDriver can be injected into Time service
void test_driver_injection_native(void) {
    // Arrange
    NativeTimeDriver native_driver;
    
    // Act
    Time::setDriver(&native_driver);
    
    // Assert
    TEST_ASSERT_EQUAL_PTR(&native_driver, Time::getDriver());
    
    // Verify the driver is being used
    std::uint32_t time1 = Time::now();
    Time::delay(10); // Small delay
    std::uint32_t time2 = Time::now();
    
    // Time should have advanced by at least 10ms
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(10, time2 - time1);
}

//! @test test_driver_no_op_behavior
//! @brief Verifies that Time service no-ops when no driver is set
void test_driver_no_op_behavior(void) {
    // Arrange
    Time::setDriver(nullptr);
    
    // Act & Assert
    TEST_ASSERT_NULL(Time::getDriver());
    
    // Verify no-op behavior
    std::uint32_t time1 = Time::now();
    Time::delay(10);
    std::uint32_t time2 = Time::now();
    
    // Time should not advance (no-op)
    TEST_ASSERT_EQUAL_UINT32(0, time1);
    TEST_ASSERT_EQUAL_UINT32(0, time2);
}

//! @test test_driver_switching
//! @brief Verifies that drivers can be switched at runtime
void test_driver_switching(void) {
    // Arrange
    NativeTimeDriver native_driver1;
    NativeTimeDriver native_driver2;
    
    // Act & Assert - Set first driver
    Time::setDriver(&native_driver1);
    TEST_ASSERT_EQUAL_PTR(&native_driver1, Time::getDriver());
    
    // Switch to second driver
    Time::setDriver(&native_driver2);
    TEST_ASSERT_EQUAL_PTR(&native_driver2, Time::getDriver());
    
    // Verify second driver works
    std::uint32_t time1 = Time::now();
    Time::delay(5);
    std::uint32_t time2 = Time::now();
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(5, time2 - time1);
}

//! @test test_driver_clear
//! @brief Verifies that driver can be cleared (set to nullptr)
void test_driver_clear(void) {
    // Arrange
    NativeTimeDriver native_driver;
    Time::setDriver(&native_driver);
    TEST_ASSERT_EQUAL_PTR(&native_driver, Time::getDriver());
    
    // Act
    Time::setDriver(nullptr);
    
    // Assert
    TEST_ASSERT_NULL(Time::getDriver());
    
    // Verify no-op behavior after clearing
    std::uint32_t time1 = Time::now();
    Time::delay(10);
    std::uint32_t time2 = Time::now();
    TEST_ASSERT_EQUAL_UINT32(0, time1);
    TEST_ASSERT_EQUAL_UINT32(0, time2);
}
