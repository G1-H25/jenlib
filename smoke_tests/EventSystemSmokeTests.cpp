//! @file smoke_tests/EventSystemSmokeTests.cpp
//! @brief Event system smoke tests for jenlib
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include "unity.h"
#include "SmokeTestSuites.h"
#include <jenlib/events/EventDispatcher.h>
#include <jenlib/events/EventTypes.h>
#include <jenlib/time/Time.h>
#include <jenlib/time/drivers/NativeTimeDriver.h>
#include <smoke_tests/PlatformMocks.h>
#include <atomic>
#include <thread>
#include <chrono>

//! @section Test State Tracking
static std::atomic<int> test_event_count{0};
static std::atomic<int> test_callback_count{0};
static std::atomic<bool> test_event_processed{false};

//! @section Test Callbacks

//! @brief Test callback for general event processing
//! @param event The event to process
void test_event_callback(const jenlib::events::Event& event) {
    test_callback_count++;
    if (event.type == jenlib::events::EventType::kTimeTick) {
        test_event_processed = true;
    }
}

//! @brief Test callback for connection state change events
//! @param event The connection state change event
void test_connection_callback(const jenlib::events::Event& event) {
    test_callback_count++;
    //! Connection state change event
}

//! @brief Test callback for BLE message events
//! @param event The BLE message event
void test_ble_message_callback(const jenlib::events::Event& event) {
    test_callback_count++;
    //! BLE message event
}

//! @section Test Setup and Teardown

//! @brief Unity test setup function - resets test state and initializes services
void setUp(void) {
    //! Reset test state
    test_event_count = 0;
    test_callback_count = 0;
    test_event_processed = false;

    //! Initialize time service with mock driver
    static smoke_tests::MockTimeDriver mock_time_driver;
    jenlib::time::Time::setDriver(&mock_time_driver);
    jenlib::time::Time::initialize();

    //! Initialize event dispatcher
    jenlib::events::EventDispatcher::initialize();
    jenlib::events::EventDispatcher::clear_all_callbacks();
}

//! @brief Unity test teardown function - cleans up after each test
void tearDown(void) {
    //! Clean up
    jenlib::events::EventDispatcher::clear_all_callbacks();
}

//! @section Event System Tests

//! @test Validates event dispatcher initialization functionality
void test_event_dispatcher_initialization(void) {
    //! ARRANGE: No setup needed - testing initialization state

    //! ACT: No action needed - testing initial state

    //! ASSERT: Verify event dispatcher is properly initialized
    TEST_ASSERT_TRUE(jenlib::events::EventDispatcher::is_initialized());
    TEST_ASSERT_EQUAL(0, jenlib::events::EventDispatcher::get_total_callback_count());
}

//! @test Validates event callback registration functionality
void test_event_callback_registration(void) {
    //! ARRANGE: No setup needed - testing registration

    //! ACT: Register a callback
    auto event_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, test_event_callback);

    //! ASSERT: Verify callback was registered successfully
    TEST_ASSERT_NOT_EQUAL(jenlib::events::kInvalidEventId, event_id);
    TEST_ASSERT_EQUAL(1, jenlib::events::EventDispatcher::get_total_callback_count());
    TEST_ASSERT_EQUAL(1, jenlib::events::EventDispatcher::get_callback_count(jenlib::events::EventType::kTimeTick));

}

//! @test Validates event dispatch functionality
void test_event_dispatch(void) {
    //! ARRANGE: Register a callback
    auto event_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, test_event_callback);
    TEST_ASSERT_NOT_EQUAL(jenlib::events::kInvalidEventId, event_id);

    //! ARRANGE: Prepare an event
    jenlib::events::Event event(jenlib::events::EventType::kTimeTick, 1000, 0);

    //! ACT: Dispatch the event
    auto dispatched_count = jenlib::events::EventDispatcher::dispatch_event(event);

    //! ASSERT: Verify event was dispatched successfully
    TEST_ASSERT_EQUAL(1, dispatched_count);
}

//! @test Validates event processing functionality
void test_event_processing(void) {
    //! ARRANGE: Register a callback and dispatch an event
    auto event_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, test_event_callback);
    jenlib::events::Event event(jenlib::events::EventType::kTimeTick, 1000, 0);
    jenlib::events::EventDispatcher::dispatch_event(event);

    //! ACT: Process events
    auto processed_count = jenlib::events::EventDispatcher::process_events();

    //! ASSERT: Verify events were processed and callbacks were called
    TEST_ASSERT_EQUAL(1, processed_count);
    TEST_ASSERT_EQUAL(1, test_callback_count.load());
    TEST_ASSERT_TRUE(test_event_processed.load());
}

//! @test Validates event callback unregistration functionality
void test_event_callback_unregistration(void) {
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

//! @test Validates event processing loop with multiple callbacks
void test_event_processing_loop(void) {
    //! ARRANGE: Register multiple callbacks
    auto time_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, test_event_callback);
    auto connection_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kConnectionStateChange, test_connection_callback);
    auto ble_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kBleMessage, test_ble_message_callback);

    TEST_ASSERT_EQUAL(3, jenlib::events::EventDispatcher::get_total_callback_count());

    //! ARRANGE: Prepare multiple events
    jenlib::events::Event time_event(jenlib::events::EventType::kTimeTick, 1000, 0);
    jenlib::events::Event connection_event(jenlib::events::EventType::kConnectionStateChange, 1001, 1);
    jenlib::events::Event ble_event(jenlib::events::EventType::kBleMessage, 1002, 0);

    //! ACT: Dispatch multiple events and process them
    jenlib::events::EventDispatcher::dispatch_event(time_event);
    jenlib::events::EventDispatcher::dispatch_event(connection_event);
    jenlib::events::EventDispatcher::dispatch_event(ble_event);

    auto processed_count = jenlib::events::EventDispatcher::process_events();

    //! ASSERT: Verify all events were processed and callbacks were called
    TEST_ASSERT_EQUAL(3, processed_count);
    TEST_ASSERT_EQUAL(3, test_callback_count.load());
}

//! @test Validates event queue overflow handling
void test_event_queue_overflow_handling(void) {
    //! ARRANGE: Register a callback
    auto event_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, test_event_callback);

    //! ACT: Dispatch more events than the queue can hold (32 max)
    for (int i = 0; i < 40; i++) {
        jenlib::events::Event event(jenlib::events::EventType::kTimeTick, 1000 + i, i);
        jenlib::events::EventDispatcher::dispatch_event(event);
    }

    //! ACT: Process events - should handle overflow gracefully
    auto processed_count = jenlib::events::EventDispatcher::process_events();

    //! ASSERT: Should process up to the queue limit
    TEST_ASSERT_LESS_OR_EQUAL(32, processed_count);
    TEST_ASSERT_GREATER_OR_EQUAL(1, processed_count);
}

//! @test Validates multiple callback registration for the same event type
void test_multiple_callback_registration(void) {
    //! ARRANGE: Register multiple callbacks for the same event type
    auto id1 = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, test_event_callback);
    auto id2 = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, test_event_callback);
    auto id3 = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, test_event_callback);

    TEST_ASSERT_EQUAL(3, jenlib::events::EventDispatcher::get_callback_count(jenlib::events::EventType::kTimeTick));
    TEST_ASSERT_EQUAL(3, jenlib::events::EventDispatcher::get_total_callback_count());

    //! ARRANGE: Prepare an event
    jenlib::events::Event event(jenlib::events::EventType::kTimeTick, 1000, 0);

    //! ACT: Dispatch one event and process it
    jenlib::events::EventDispatcher::dispatch_event(event);
    auto processed_count = jenlib::events::EventDispatcher::process_events();

    //! ASSERT: Should invoke all 3 callbacks
    TEST_ASSERT_EQUAL(3, processed_count);
    TEST_ASSERT_EQUAL(3, test_callback_count.load());
}

//! @section Test Runner

//! @brief Main function to run all event system smoke tests
int main(void) {
    UNITY_BEGIN();

    // Event Dispatcher Initialization Tests
    RUN_TEST(test_event_dispatcher_initialization);

    // Event Callback Management Tests
    RUN_TEST(test_event_callback_registration);
    RUN_TEST(test_event_callback_unregistration);

    // Event Processing Tests
    RUN_TEST(test_event_dispatch);
    RUN_TEST(test_event_processing);
    RUN_TEST(test_event_processing_loop);

    // Event System Edge Case Tests
    RUN_TEST(test_event_queue_overflow_handling);
    RUN_TEST(test_multiple_callback_registration);

    return UNITY_END();
}

