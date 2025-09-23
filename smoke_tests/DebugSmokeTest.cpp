//! @file smoke_tests/DebugSmokeTest.cpp
//! @brief Debug smoke test to isolate event processing issues
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

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

//! @section Debug Smoke Tests

//! @test Validates event dispatcher initialization for debugging
void test_event_dispatcher_initialization_debug(void) {
    //! ARRANGE: No setup needed - testing initialization state

    //! ACT: No action needed - testing initial state

    //! ASSERT: Verify event dispatcher is properly initialized
    TEST_ASSERT_TRUE(jenlib::events::EventDispatcher::is_initialized());
    TEST_ASSERT_EQUAL(0, jenlib::events::EventDispatcher::get_total_callback_count());
}

//! @test Validates event dispatcher callback registration for debugging
void test_event_dispatcher_callback_registration_debug(void) {
    //! ARRANGE: No setup needed - using initialized event dispatcher

    //! ACT: Register a callback
    auto event_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, test_event_callback);

    //! ASSERT: Verify callback was registered successfully
    TEST_ASSERT_NOT_EQUAL(jenlib::events::kInvalidEventId, event_id);
    TEST_ASSERT_EQUAL(1, jenlib::events::EventDispatcher::get_total_callback_count());
    TEST_ASSERT_EQUAL(1, jenlib::events::EventDispatcher::get_callback_count(jenlib::events::EventType::kTimeTick));
}

//! @test Validates event dispatcher event dispatch and processing for debugging
void test_event_dispatcher_event_processing_debug(void) {
    //! ARRANGE: Register a callback
    auto event_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, test_event_callback);
    TEST_ASSERT_NOT_EQUAL(jenlib::events::kInvalidEventId, event_id);

    //! ACT: Dispatch and process an event
    jenlib::events::Event event(jenlib::events::EventType::kTimeTick, 1000, 0);
    auto dispatched_count = jenlib::events::EventDispatcher::dispatch_event(event);
    auto processed_count = jenlib::events::EventDispatcher::process_events();

    //! ASSERT: Verify event was dispatched and processed correctly
    TEST_ASSERT_EQUAL(1, dispatched_count);
    TEST_ASSERT_EQUAL(1, processed_count);
    TEST_ASSERT_EQUAL(1, test_callback_count.load());
}

//! @test Validates event dispatcher callback unregistration for debugging
void test_event_dispatcher_callback_unregistration_debug(void) {
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

//! @test Validates multiple event type callback registration for debugging
void test_multiple_event_types_callback_registration_debug(void) {
    //! ARRANGE: Prepare callback counters
    std::atomic<int> time_tick_count{0};
    std::atomic<int> connection_count{0};
    std::atomic<int> ble_message_count{0};

    //! ACT: Register callbacks for different event types
    auto time_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, [&time_tick_count](const jenlib::events::Event&) {
            time_tick_count++;
        });

    auto connection_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kConnectionStateChange, [&connection_count](const jenlib::events::Event&) {
            connection_count++;
        });

    auto ble_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kBleMessage, [&ble_message_count](const jenlib::events::Event&) {
            ble_message_count++;
        });

    //! ASSERT: Verify all callbacks were registered successfully
    TEST_ASSERT_EQUAL(3, jenlib::events::EventDispatcher::get_total_callback_count());
}

//! @test Validates multiple event type dispatch and processing for debugging
void test_multiple_event_types_processing_debug(void) {
    //! ARRANGE: Register callbacks for different event types
    std::atomic<int> time_tick_count{0};
    std::atomic<int> connection_count{0};
    std::atomic<int> ble_message_count{0};

    auto time_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, [&time_tick_count](const jenlib::events::Event&) {
            time_tick_count++;
        });

    auto connection_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kConnectionStateChange, [&connection_count](const jenlib::events::Event&) {
            connection_count++;
        });

    auto ble_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kBleMessage, [&ble_message_count](const jenlib::events::Event&) {
            ble_message_count++;
        });

    //! ACT: Dispatch different event types and process them
    jenlib::events::Event time_event(jenlib::events::EventType::kTimeTick, 1000, 0);
    jenlib::events::Event connection_event(jenlib::events::EventType::kConnectionStateChange, 1001, 1);
    jenlib::events::Event ble_event(jenlib::events::EventType::kBleMessage, 1002, 0);

    jenlib::events::EventDispatcher::dispatch_event(time_event);
    jenlib::events::EventDispatcher::dispatch_event(connection_event);
    jenlib::events::EventDispatcher::dispatch_event(ble_event);

    auto processed_count = jenlib::events::EventDispatcher::process_events();

    //! ASSERT: Verify all events were processed correctly
    TEST_ASSERT_EQUAL(3, processed_count);
    TEST_ASSERT_EQUAL(1, time_tick_count.load());
    TEST_ASSERT_EQUAL(1, connection_count.load());
    TEST_ASSERT_EQUAL(1, ble_message_count.load());

    //! CLEANUP: Unregister callbacks
    jenlib::events::EventDispatcher::unregister_callback(time_id);
    jenlib::events::EventDispatcher::unregister_callback(connection_id);
    jenlib::events::EventDispatcher::unregister_callback(ble_id);
}

//! @section Test Runner

//! @brief Main function to run all debug smoke tests
int main() {
    UNITY_BEGIN();

    // Event Dispatcher Debug Tests
    RUN_TEST(test_event_dispatcher_initialization_debug);
    RUN_TEST(test_event_dispatcher_callback_registration_debug);
    RUN_TEST(test_event_dispatcher_event_processing_debug);
    RUN_TEST(test_event_dispatcher_callback_unregistration_debug);

    // Multiple Event Types Debug Tests
    RUN_TEST(test_multiple_event_types_callback_registration_debug);
    RUN_TEST(test_multiple_event_types_processing_debug);

    return UNITY_END();
}

