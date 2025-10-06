//! @file smoke_tests/SimpleSmokeTest.cpp
//! @brief Simple smoke test to verify basic functionality
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <unity.h>
#include <atomic>
#include "jenlib/events/EventDispatcher.h"
#include "jenlib/events/EventTypes.h"
#include "jenlib/time/Time.h"
#include "jenlib/time/drivers/NativeTimeDriver.h"

//! @section Test State Tracking
static std::atomic<int> test_callback_count{0};

//! @section Test Callbacks

//! @brief Test callback for event processing validation
//! @param event The event that triggered the callback
void test_event_callback(const jenlib::events::Event& event) {
    test_callback_count++;
}

//! @section Test Setup and Teardown

//! @brief Unity test setup function - resets test state and initializes services
void setUp(void) {
    //! ARRANGE: Reset test state
    test_callback_count = 0;

    //! ARRANGE: Initialize time service with native driver
    static jenlib::time::NativeTimeDriver native_time_driver;
    jenlib::time::Time::setDriver(&native_time_driver);
    jenlib::time::Time::initialize();

    //! ARRANGE: Initialize event dispatcher
    jenlib::events::EventDispatcher::initialize();
    jenlib::events::EventDispatcher::clear_all_callbacks();
}

//! @brief Unity test teardown function - cleans up after each test
void tearDown(void) {
    //! CLEANUP: Clean up event dispatcher
    jenlib::events::EventDispatcher::clear_all_callbacks();

    //! CLEANUP: Clean up time service
    jenlib::time::Time::clear_all_timers();
}

//! @section Simple Smoke Tests

//! @test Validates event dispatcher initialization functionality
void test_event_dispatcher_initialization(void) {
    //! ARRANGE: No setup needed - testing initialization state

    //! ACT: No action needed - testing initial state

    //! ASSERT: Verify event dispatcher is properly initialized
    TEST_ASSERT_TRUE(jenlib::events::EventDispatcher::is_initialized());
    TEST_ASSERT_EQUAL(0, jenlib::events::EventDispatcher::get_total_callback_count());
}

//! @test Validates event dispatcher callback registration functionality
void test_event_dispatcher_callback_registration(void) {
    //! ARRANGE: No setup needed - using initialized event dispatcher

    //! ACT: Register a callback
    auto event_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, test_event_callback);

    //! ASSERT: Verify callback was registered successfully
    TEST_ASSERT_NOT_EQUAL(jenlib::events::kInvalidEventId, event_id);
    TEST_ASSERT_EQUAL(1, jenlib::events::EventDispatcher::get_total_callback_count());
}

//! @test Validates event dispatcher event dispatch and processing functionality
void test_event_dispatcher_event_processing(void) {
    //! ARRANGE: Register a callback
    auto event_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, test_event_callback);
    TEST_ASSERT_NOT_EQUAL(jenlib::events::kInvalidEventId, event_id);

    //! ACT: Dispatch and process an event
    jenlib::events::Event event(jenlib::events::EventType::kTimeTick, 1000, 0);
    auto enqueue_result = jenlib::events::EventDispatcher::dispatch_event(event);
    auto processed_count = jenlib::events::EventDispatcher::process_events();

    //! ASSERT: Verify event was dispatched and processed correctly
    TEST_ASSERT_EQUAL(static_cast<int>(jenlib::events::EventEnqueueResult::Enqueued),
                      static_cast<int>(enqueue_result));
    TEST_ASSERT_EQUAL(1, processed_count);
    TEST_ASSERT_EQUAL(1, test_callback_count.load());
}

//! @test Validates event dispatcher callback unregistration functionality
void test_event_dispatcher_callback_unregistration(void) {
    //! ARRANGE: Register a callback
    auto event_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, test_event_callback);
    TEST_ASSERT_EQUAL(1, jenlib::events::EventDispatcher::get_total_callback_count());

    //! ACT: Unregister the callback
    bool unregistered = jenlib::events::EventDispatcher::unregister_callback(event_id);

    //! ASSERT: Verify callback was unregistered successfully
    TEST_ASSERT_TRUE(unregistered);
    TEST_ASSERT_EQUAL(0, jenlib::events::EventDispatcher::get_total_callback_count());
}

//! @test Validates time service initialization functionality
void test_time_service_initialization(void) {
    //! ARRANGE: No setup needed - testing initialization state

    //! ACT: No action needed - testing initial state

    //! ASSERT: Verify time service is properly initialized
    TEST_ASSERT_TRUE(jenlib::time::Time::is_initialized());
    TEST_ASSERT_EQUAL(0, jenlib::time::Time::get_active_timer_count());
}

//! @test Validates time service basic time operations
void test_time_service_basic_operations(void) {
    //! ARRANGE: No setup needed - using initialized time service

    //! ACT: Get current time
    auto current_time = jenlib::time::Time::now();

    //! ASSERT: Verify time operations work correctly
    TEST_ASSERT_GREATER_OR_EQUAL(0, current_time);
}

//! @test Validates time service timer scheduling and execution functionality
void test_time_service_timer_scheduling(void) {
    //! ARRANGE: Prepare timer callback
    std::atomic<bool> timer_fired{false};

    //! ACT: Schedule a timer
    auto timer_id = jenlib::time::Time::schedule_callback(100, [&timer_fired]() {
        timer_fired = true;
    }, false);

    //! ASSERT: Verify timer was scheduled correctly
    TEST_ASSERT_NOT_EQUAL(jenlib::time::kInvalidTimerId, timer_id);
    TEST_ASSERT_EQUAL(1, jenlib::time::Time::get_active_timer_count());
}

//! @test Validates time service timer execution and cleanup functionality
void test_time_service_timer_execution(void) {
    //! ARRANGE: Schedule a timer
    std::atomic<bool> timer_fired{false};
    auto timer_id = jenlib::time::Time::schedule_callback(100, [&timer_fired]() {
        timer_fired = true;
    }, false);
    TEST_ASSERT_NOT_EQUAL(jenlib::time::kInvalidTimerId, timer_id);

    //! ACT: Wait for timer and process timers
    jenlib::time::Time::delay(150);
    auto fired_count = jenlib::time::Time::process_timers();

    //! ASSERT: Verify timer executed correctly and was cleaned up
    TEST_ASSERT_EQUAL(1, fired_count);
    TEST_ASSERT_TRUE(timer_fired.load());
    TEST_ASSERT_EQUAL(0, jenlib::time::Time::get_active_timer_count());
}

//! @section Test Runner

//! @brief Main function to run all simple smoke tests
int main() {
    UNITY_BEGIN();

    // Event Dispatcher Tests
    RUN_TEST(test_event_dispatcher_initialization);
    RUN_TEST(test_event_dispatcher_callback_registration);
    RUN_TEST(test_event_dispatcher_event_processing);
    RUN_TEST(test_event_dispatcher_callback_unregistration);

    // Time Service Tests
    RUN_TEST(test_time_service_initialization);
    RUN_TEST(test_time_service_basic_operations);
    RUN_TEST(test_time_service_timer_scheduling);
    RUN_TEST(test_time_service_timer_execution);

    return UNITY_END();
}

