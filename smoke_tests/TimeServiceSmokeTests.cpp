//! @file smoke_tests/TimeServiceSmokeTests.cpp
//! @brief Time service smoke tests for jenlib
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include "unity.h"
#include "SmokeTestSuites.h"
#include <jenlib/time/Time.h>
#include <jenlib/time/drivers/NativeTimeDriver.h>
#include <smoke_tests/PlatformMocks.h>
#include <atomic>
#include <thread>
#include <chrono>

//! @section Test State Tracking
static std::atomic<int> timer_callback_count{0};
static std::atomic<int> repeating_timer_count{0};
static std::atomic<bool> one_shot_timer_fired{false};
static std::atomic<bool> repeating_timer_fired{false};

//! @section Test Callbacks

//! @brief Test callback for general timer functionality
void test_timer_callback() {
    timer_callback_count++;
}

//! @brief Test callback for repeating timer functionality
void test_repeating_timer_callback() {
    repeating_timer_count++;
    repeating_timer_fired = true;
}

//! @brief Test callback for one-shot timer functionality
void test_one_shot_timer_callback() {
    one_shot_timer_fired = true;
}

//! @section Test Setup and Teardown

//! @brief Unity test setup function - resets test state and initializes time service
void setUp(void) {
    //! Reset test state
    timer_callback_count = 0;
    repeating_timer_count = 0;
    one_shot_timer_fired = false;
    repeating_timer_fired = false;
    
    //! Initialize time service with mock driver
    static smoke_tests::MockTimeDriver mock_time_driver;
    jenlib::time::Time::setDriver(&mock_time_driver);
    jenlib::time::Time::initialize();
    jenlib::time::Time::clear_all_timers();
}

//! @brief Unity test teardown function - cleans up after each test
void tearDown(void) {
    //! Clean up
    jenlib::time::Time::clear_all_timers();
}

//! @section Time Service Tests

//! @test Validates time service initialization functionality
void test_time_service_initialization(void) {
    //! ARRANGE: No setup needed - testing initialization state
    
    //! ACT: No action needed - testing initial state
    
    //! ASSERT: Verify time service is properly initialized
    TEST_ASSERT_TRUE(jenlib::time::Time::is_initialized());
    TEST_ASSERT_EQUAL(0, jenlib::time::Time::get_active_timer_count());
    TEST_ASSERT_EQUAL(0, jenlib::time::Time::get_total_timer_count());
}

//! @test Validates timer scheduling functionality
void test_timer_scheduling(void) {
    //! ARRANGE: No setup needed - testing scheduling
    
    //! ACT: Schedule a timer
    auto timer_id = jenlib::time::Time::schedule_callback(1000, test_timer_callback, false);
    
    //! ASSERT: Verify timer was scheduled successfully
    TEST_ASSERT_NOT_EQUAL(jenlib::time::kInvalidTimerId, timer_id);
    TEST_ASSERT_EQUAL(1, jenlib::time::Time::get_active_timer_count());
    TEST_ASSERT_EQUAL(1, jenlib::time::Time::get_total_timer_count());
}

//! @test Validates timer cancellation functionality
void test_timer_cancellation(void) {
    //! ARRANGE: Schedule a timer
    auto timer_id = jenlib::time::Time::schedule_callback(1000, test_timer_callback, false);
    TEST_ASSERT_EQUAL(1, jenlib::time::Time::get_active_timer_count());
    
    //! ACT: Cancel the timer
    bool cancelled = jenlib::time::Time::cancel_callback(timer_id);
    
    //! ASSERT: Verify timer was cancelled successfully
    TEST_ASSERT_TRUE(cancelled);
    TEST_ASSERT_EQUAL(0, jenlib::time::Time::get_active_timer_count());
    TEST_ASSERT_EQUAL(0, jenlib::time::Time::get_total_timer_count());
}

//! @test Validates timer processing loop functionality
void test_timer_processing_loop(void) {
    //! ARRANGE: Schedule a timer
    auto timer_id = jenlib::time::Time::schedule_callback(1000, test_timer_callback, false);
    TEST_ASSERT_EQUAL(1, jenlib::time::Time::get_active_timer_count());
    
    //! ACT: Advance time to trigger timer and process timers
    static_cast<smoke_tests::MockTimeDriver*>(jenlib::time::Time::getDriver())->advance_time(1000);
    auto fired_count = jenlib::time::Time::process_timers();
    
    //! ASSERT: Verify timer fired and callback was called
    TEST_ASSERT_EQUAL(1, fired_count);
    TEST_ASSERT_EQUAL(1, timer_callback_count.load());
    TEST_ASSERT_EQUAL(0, jenlib::time::Time::get_active_timer_count()); // One-shot timer should be inactive
}

//! @test Validates repeating timer functionality
void test_repeating_timer_functionality(void) {
    //! ARRANGE: Schedule a repeating timer
    auto timer_id = jenlib::time::Time::schedule_callback(500, test_repeating_timer_callback, true);
    TEST_ASSERT_NOT_EQUAL(jenlib::time::kInvalidTimerId, timer_id);
    TEST_ASSERT_EQUAL(1, jenlib::time::Time::get_active_timer_count());
    
    //! ACT: Advance time and process multiple times
    auto mock_driver = static_cast<smoke_tests::MockTimeDriver*>(jenlib::time::Time::getDriver());
    
    for (int i = 0; i < 5; i++) {
        mock_driver->advance_time(500);
        auto fired_count = jenlib::time::Time::process_timers();
        
        TEST_ASSERT_EQUAL(1, fired_count);
        TEST_ASSERT_EQUAL(1, jenlib::time::Time::get_active_timer_count()); // Repeating timer should stay active
    }
    
    //! ASSERT: Verify repeating timer fired multiple times
    TEST_ASSERT_EQUAL(5, repeating_timer_count.load());
    TEST_ASSERT_TRUE(repeating_timer_fired.load());
}

//! @test Validates repeating timer cancellation functionality
void test_repeating_timer_cancellation(void) {
    //! ARRANGE: Schedule a repeating timer
    auto timer_id = jenlib::time::Time::schedule_callback(500, test_repeating_timer_callback, true);
    TEST_ASSERT_EQUAL(1, jenlib::time::Time::get_active_timer_count());
    
    //! ACT: Cancel the repeating timer
    bool cancelled = jenlib::time::Time::cancel_callback(timer_id);
    
    //! ASSERT: Verify timer was cancelled successfully
    TEST_ASSERT_TRUE(cancelled);
    TEST_ASSERT_EQUAL(0, jenlib::time::Time::get_active_timer_count());
}

//! @test Validates one-shot timer functionality
void test_one_shot_timer_functionality(void) {
    //! ARRANGE: Schedule a one-shot timer
    auto timer_id = jenlib::time::Time::schedule_callback(1000, test_one_shot_timer_callback, false);
    TEST_ASSERT_NOT_EQUAL(jenlib::time::kInvalidTimerId, timer_id);
    TEST_ASSERT_EQUAL(1, jenlib::time::Time::get_active_timer_count());
    
    //! ACT: Advance time to trigger timer and process timers
    auto mock_driver = static_cast<smoke_tests::MockTimeDriver*>(jenlib::time::Time::getDriver());
    mock_driver->advance_time(1000);
    auto fired_count = jenlib::time::Time::process_timers();
    
    //! ASSERT: Verify one-shot timer fired and became inactive
    TEST_ASSERT_EQUAL(1, fired_count);
    TEST_ASSERT_TRUE(one_shot_timer_fired.load());
    TEST_ASSERT_EQUAL(0, jenlib::time::Time::get_active_timer_count()); // One-shot should be inactive
    
    //! ACT: Try to process again - should not fire
    fired_count = jenlib::time::Time::process_timers();
    
    //! ASSERT: Verify timer does not fire again
    TEST_ASSERT_EQUAL(0, fired_count);
}

//! @test Validates timer overflow handling functionality
void test_timer_overflow_handling(void) {
    //! ARRANGE: Prepare to test timer overflow
    std::vector<jenlib::time::TimerId> timer_ids;
    
    //! ACT: Try to schedule more timers than the maximum (16)
    for (int i = 0; i < 20; i++) {
        auto timer_id = jenlib::time::Time::schedule_callback(1000 + i, test_timer_callback, false);
        if (timer_id != jenlib::time::kInvalidTimerId) {
            timer_ids.push_back(timer_id);
        }
    }
    
    //! ASSERT: Should have scheduled up to the maximum number of timers
    TEST_ASSERT_LESS_OR_EQUAL(16, timer_ids.size());
    TEST_ASSERT_GREATER_OR_EQUAL(1, timer_ids.size());
    TEST_ASSERT_EQUAL(timer_ids.size(), jenlib::time::Time::get_active_timer_count());
}

//! @section Test Runner

//! @brief Main function to run all time service smoke tests
int main(void) {
    UNITY_BEGIN();
    
    // Time Service Initialization Tests
    RUN_TEST(test_time_service_initialization);
    
    // Timer Management Tests
    RUN_TEST(test_timer_scheduling);
    RUN_TEST(test_timer_cancellation);
    
    // Timer Processing Tests
    RUN_TEST(test_timer_processing_loop);
    
    // Timer Type Tests
    RUN_TEST(test_repeating_timer_functionality);
    RUN_TEST(test_repeating_timer_cancellation);
    RUN_TEST(test_one_shot_timer_functionality);
    
    // Timer Edge Case Tests
    RUN_TEST(test_timer_overflow_handling);
    
    return UNITY_END();
}
