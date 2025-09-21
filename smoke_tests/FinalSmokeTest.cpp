//! @file smoke_tests/FinalSmokeTest.cpp
//! @brief Comprehensive smoke test suite following AAA patterns with enhanced correctness validation
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include "unity.h"
#include <jenlib/events/EventDispatcher.h>
#include <jenlib/events/EventTypes.h>
#include <jenlib/time/Time.h>
#include <jenlib/time/drivers/NativeTimeDriver.h>
#include <jenlib/state/SensorStateMachine.h>
#include <jenlib/measurement/Measurement.h>
#include <atomic>
#include <vector>
#include <functional>

//! @section Test State Tracking
static std::atomic<int> connection_events{0};
static std::atomic<int> ble_message_events{0};
static std::atomic<int> time_tick_events{0};

//! @section Validation Helper Functions

//! @brief Validates sensor state machine state and invariants
//! @param state_machine The state machine to validate
//! @param expected_state The expected current state
//! @param expected_session_active Whether session should be active
//! @param expected_session_id The expected session ID (0 if no session)
void validate_sensor_state_machine_state(
    const jenlib::state::SensorStateMachine& state_machine,
    jenlib::state::SensorState expected_state,
    bool expected_session_active,
    std::uint32_t expected_session_id = 0) {
    
    TEST_ASSERT_EQUAL(expected_state, state_machine.get_current_state());
    TEST_ASSERT_TRUE(state_machine.is_in_state(expected_state));
    TEST_ASSERT_EQUAL(expected_session_active, state_machine.is_session_active());
    TEST_ASSERT_EQUAL(expected_session_id, state_machine.get_current_session_id().value());
}

//! @brief Validates event system callback registration
//! @param expected_count Expected number of registered callbacks
void validate_event_system_callbacks(int expected_count) {
    TEST_ASSERT_EQUAL(expected_count, jenlib::events::EventDispatcher::get_total_callback_count());
}

//! @brief Validates timer system state
//! @param expected_active_count Expected number of active timers
void validate_timer_system_state(int expected_active_count) {
    TEST_ASSERT_EQUAL(expected_active_count, jenlib::time::Time::get_active_timer_count());
}

//! @section Event Handlers (Simulating Example Main.cpp)

//! @brief Event handler for time tick events
void test_handle_time_tick_event(const jenlib::events::Event& event) {
    time_tick_events++;
}

//! @brief Event handler for BLE message events
void test_handle_ble_message_event(const jenlib::events::Event& event) {
    ble_message_events++;
}

//! @brief Event handler for connection state change events
void test_handle_connection_state_event(const jenlib::events::Event& event) {
    connection_events++;
}

//! @section Test Setup and Teardown

//! @brief Unity test setup function - initializes all systems for each test
void setUp(void) {
    //! Reset test state
    connection_events = 0;
    ble_message_events = 0;
    time_tick_events = 0;
    
    //! Initialize time service
    static jenlib::time::NativeTimeDriver native_time_driver;
    jenlib::time::Time::setDriver(&native_time_driver);
    jenlib::time::Time::initialize();
    jenlib::time::Time::clear_all_timers();
    
    //! Initialize event dispatcher
    jenlib::events::EventDispatcher::initialize();
    jenlib::events::EventDispatcher::clear_all_callbacks();
}

//! @brief Unity test teardown function - cleans up all systems after each test
void tearDown(void) {
    //! Reset all systems
    jenlib::events::EventDispatcher::clear_all_callbacks();
    jenlib::time::Time::clear_all_timers();
}

//! @section State Machine Tests

//! @test Validates that sensor state machine starts in disconnected state
void test_sensor_state_machine_initial_state(void) {
    //! ARRANGE: Create state machine
    jenlib::state::SensorStateMachine state_machine;
    
    //! ACT: No action needed - testing initial state
    
    //! ASSERT: Verify initial state
    validate_sensor_state_machine_state(
        state_machine, 
        jenlib::state::SensorState::kDisconnected, 
        false,  // session not active
        0       // no session ID
    );
}

//! @test Validates connection transition from disconnected to waiting state
void test_sensor_state_machine_connection_transition(void) {
    //! ARRANGE: Create state machine and verify initial state
    jenlib::state::SensorStateMachine state_machine;
    validate_sensor_state_machine_state(
        state_machine, 
        jenlib::state::SensorState::kDisconnected, 
        false, 0
    );
    
    //! ACT: Trigger connection change
    bool transitioned = state_machine.handle_connection_change(true);
    
    //! ASSERT: Verify transition to waiting state
    TEST_ASSERT_TRUE(transitioned);
    validate_sensor_state_machine_state(
        state_machine, 
        jenlib::state::SensorState::kWaiting, 
        false,  // session still not active
        0       // no session ID yet
    );
}

//! @test Validates session start transition from waiting to running state
void test_sensor_state_machine_session_start_transition(void) {
    //! ARRANGE: Set up connected state and prepare start message
    jenlib::state::SensorStateMachine state_machine;
    state_machine.handle_connection_change(true);
    validate_sensor_state_machine_state(
        state_machine, 
        jenlib::state::SensorState::kWaiting, 
        false, 0
    );
    
    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };
    
    //! ACT: Handle start broadcast message
    bool started = state_machine.handle_start_broadcast(
        jenlib::ble::DeviceId(0x87654321), start_msg);
    
    //! ASSERT: Verify transition to running state with active session
    TEST_ASSERT_TRUE(started);
    validate_sensor_state_machine_state(
        state_machine, 
        jenlib::state::SensorState::kRunning, 
        true,   // session now active
        0x1234  // session ID set
    );
}

//! @test Validates session end transition from running to waiting state
void test_sensor_state_machine_session_end_transition(void) {
    //! ARRANGE: Set up running session state
    jenlib::state::SensorStateMachine state_machine;
    state_machine.handle_connection_change(true);
    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };
    state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);
    
    validate_sensor_state_machine_state(
        state_machine, 
        jenlib::state::SensorState::kRunning, 
        true, 0x1234
    );
    
    //! ACT: Handle session end
    bool ended = state_machine.handle_session_end();
    
    //! ASSERT: Verify transition to waiting state with session cleared
    TEST_ASSERT_TRUE(ended);
    validate_sensor_state_machine_state(
        state_machine, 
        jenlib::state::SensorState::kWaiting, 
        false,  // session no longer active
        0       // session ID cleared
    );
}

//! @test Validates disconnection transition from waiting to disconnected state
void test_sensor_state_machine_disconnection_transition(void) {
    //! ARRANGE: Set up waiting state (connected but no session)
    jenlib::state::SensorStateMachine state_machine;
    state_machine.handle_connection_change(true);
    validate_sensor_state_machine_state(
        state_machine, 
        jenlib::state::SensorState::kWaiting, 
        false, 0
    );
    
    //! ACT: Trigger disconnection
    bool disconnected = state_machine.handle_connection_change(false);
    
    //! ASSERT: Verify transition to disconnected state
    TEST_ASSERT_TRUE(disconnected);
    validate_sensor_state_machine_state(
        state_machine, 
        jenlib::state::SensorState::kDisconnected, 
        false, 0
    );
}

//! @section Event System Tests

//! @test Validates event system callback registration functionality
void test_event_system_callback_registration(void) {
    //! ARRANGE: Prepare event counter
    std::atomic<int> event_count{0};
    
    //! ACT: Register callbacks for different event types
    auto time_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, [&event_count](const jenlib::events::Event&) {
            event_count++;
        });
    
    auto connection_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kConnectionStateChange, [&event_count](const jenlib::events::Event&) {
            event_count++;
        });
    
    auto ble_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kBleMessage, [&event_count](const jenlib::events::Event&) {
            event_count++;
        });
    
    //! ASSERT: Verify all callbacks were registered
    validate_event_system_callbacks(3);
    
    //! CLEANUP: Unregister all callbacks
    jenlib::events::EventDispatcher::unregister_callback(time_id);
    jenlib::events::EventDispatcher::unregister_callback(connection_id);
    jenlib::events::EventDispatcher::unregister_callback(ble_id);
}

//! @test Validates event dispatch and processing functionality
void test_event_system_event_dispatch_and_processing(void) {
    //! ARRANGE: Register callbacks and prepare test events
    std::atomic<int> event_count{0};
    
    auto time_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick, [&event_count](const jenlib::events::Event&) {
            event_count++;
        });
    
    auto connection_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kConnectionStateChange, [&event_count](const jenlib::events::Event&) {
            event_count++;
        });
    
    auto ble_id = jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kBleMessage, [&event_count](const jenlib::events::Event&) {
            event_count++;
        });
    
    jenlib::events::Event time_event(jenlib::events::EventType::kTimeTick, 1000, 0);
    jenlib::events::Event connection_event(jenlib::events::EventType::kConnectionStateChange, 1001, 1);
    jenlib::events::Event ble_event(jenlib::events::EventType::kBleMessage, 1002, 0);
    
    //! ACT: Dispatch events and process them
    jenlib::events::EventDispatcher::dispatch_event(time_event);
    jenlib::events::EventDispatcher::dispatch_event(connection_event);
    jenlib::events::EventDispatcher::dispatch_event(ble_event);
    
    auto processed_count = jenlib::events::EventDispatcher::process_events();
    
    //! ASSERT: Verify all events were processed and callbacks invoked
    TEST_ASSERT_EQUAL(3, processed_count);
    TEST_ASSERT_EQUAL(3, event_count.load());
    
    //! CLEANUP: Unregister all callbacks
    jenlib::events::EventDispatcher::unregister_callback(time_id);
    jenlib::events::EventDispatcher::unregister_callback(connection_id);
    jenlib::events::EventDispatcher::unregister_callback(ble_id);
}

//! @section Measurement Conversion Tests

//! @test Validates temperature conversion accuracy across various values
void test_temperature_conversion_accuracy(void) {
    //! ARRANGE: Define test data and expected results
    const float test_temps[] = {22.5f, -10.0f, 0.0f, 100.0f};
    const std::int16_t expected_centi[] = {2250, -1000, 0, 10000};
    
    //! ACT & ASSERT: Test each temperature conversion
    for (size_t i = 0; i < 4; ++i) {
        std::int16_t result = measurement::temperature_to_centi(test_temps[i]);
        TEST_ASSERT_EQUAL(expected_centi[i], result);
    }
}

//! @test Validates humidity conversion accuracy across various values
void test_humidity_conversion_accuracy(void) {
    //! ARRANGE: Define test data and expected results
    const float test_humidity[] = {45.0f, 0.0f, 100.0f, 50.5f};
    const std::uint16_t expected_basis_points[] = {4500, 0, 10000, 5050};
    
    //! ACT & ASSERT: Test each humidity conversion
    for (size_t i = 0; i < 4; ++i) {
        std::uint16_t result = measurement::humidity_to_basis_points(test_humidity[i]);
        TEST_ASSERT_EQUAL(expected_basis_points[i], result);
    }
}

//! @section Timer System Tests

//! @test Validates timer scheduling and execution functionality
void test_timer_scheduling_and_execution(void) {
    //! ARRANGE: Prepare timer callback and delay
    std::atomic<bool> timer_fired{false};
    const int timer_delay_ms = 100;
    
    //! ACT: Schedule timer
    auto timer_id = jenlib::time::Time::schedule_callback(
        timer_delay_ms, [&timer_fired]() {
            timer_fired = true;
        }, false);
    
    //! ASSERT: Verify timer was scheduled
    TEST_ASSERT_NOT_EQUAL(jenlib::time::kInvalidTimerId, timer_id);
    validate_timer_system_state(1);
    
    //! ACT: Wait for timer and process
    jenlib::time::Time::delay(timer_delay_ms + 50);  // Wait longer than timer delay
    auto fired_count = jenlib::time::Time::process_timers();
    
    //! ASSERT: Verify timer fired and was processed
    TEST_ASSERT_EQUAL(1, fired_count);
    TEST_ASSERT_TRUE(timer_fired.load());
    validate_timer_system_state(0);
}

//! @test Validates timer cancellation functionality
void test_timer_cancellation(void) {
    //! ARRANGE: Prepare timer callback and delay
    std::atomic<bool> timer_fired{false};
    const int timer_delay_ms = 100;
    
    //! ACT: Schedule timer
    auto timer_id = jenlib::time::Time::schedule_callback(
        timer_delay_ms, [&timer_fired]() {
            timer_fired = true;
        }, false);
    
    TEST_ASSERT_NOT_EQUAL(jenlib::time::kInvalidTimerId, timer_id);
    validate_timer_system_state(1);
    
    //! ACT: Cancel the timer
    bool cancelled = jenlib::time::Time::cancel_callback(timer_id);
    
    //! ASSERT: Verify timer was cancelled
    TEST_ASSERT_TRUE(cancelled);
    validate_timer_system_state(0);
    
    //! ACT & ASSERT: Wait and verify timer didn't fire
    jenlib::time::Time::delay(timer_delay_ms + 50);
    auto fired_count = jenlib::time::Time::process_timers();
    
    TEST_ASSERT_EQUAL(0, fired_count);
    TEST_ASSERT_FALSE(timer_fired.load());
}

//! @section Integration Tests

//! @test Validates complete sensor lifecycle from connection to disconnection
void test_full_sensor_lifecycle_integration(void) {
    //! ARRANGE: Create state machine and verify initial state
    jenlib::state::SensorStateMachine state_machine;
    validate_sensor_state_machine_state(
        state_machine, 
        jenlib::state::SensorState::kDisconnected, 
        false, 0
    );
    
    //! ACT & ASSERT: Connection Phase
    bool connected = state_machine.handle_connection_change(true);
    TEST_ASSERT_TRUE(connected);
    validate_sensor_state_machine_state(
        state_machine, 
        jenlib::state::SensorState::kWaiting, 
        false, 0
    );
    
    //! ACT & ASSERT: Session Start Phase
    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };
    
    bool started = state_machine.handle_start_broadcast(
        jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_TRUE(started);
    validate_sensor_state_machine_state(
        state_machine, 
        jenlib::state::SensorState::kRunning, 
        true, 0x1234
    );
    
    //! ACT & ASSERT: Measurement Phase
    bool measurement_handled = state_machine.handle_measurement_timer();
    TEST_ASSERT_TRUE(measurement_handled);
    
    //! ACT & ASSERT: Receipt Phase
    jenlib::ble::ReceiptMsg receipt_msg{
        .session_id = jenlib::ble::SessionId(0x1234),
        .up_to_offset_ms = 1000
    };
    
    bool receipt_handled = state_machine.handle_receipt(
        jenlib::ble::DeviceId(0x87654321), receipt_msg);
    TEST_ASSERT_TRUE(receipt_handled);
    
    //! ACT & ASSERT: Session End Phase
    bool ended = state_machine.handle_session_end();
    TEST_ASSERT_TRUE(ended);
    validate_sensor_state_machine_state(
        state_machine, 
        jenlib::state::SensorState::kWaiting, 
        false, 0
    );
    
    //! ACT & ASSERT: Disconnection Phase
    bool disconnected = state_machine.handle_connection_change(false);
    TEST_ASSERT_TRUE(disconnected);
    validate_sensor_state_machine_state(
        state_machine, 
        jenlib::state::SensorState::kDisconnected, 
        false, 0
    );
}

//! @section Error Handling Tests

//! @test Validates state machine rejects invalid transitions
void test_state_machine_invalid_transitions(void) {
    //! ARRANGE: Create state machine in disconnected state
    jenlib::state::SensorStateMachine state_machine;
    
    //! ACT & ASSERT: Try to start session without connection
    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };
    
    bool started = state_machine.handle_start_broadcast(
        jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_FALSE(started);  // Should fail - not connected
    
    //! ASSERT: Verify state unchanged
    validate_sensor_state_machine_state(
        state_machine, 
        jenlib::state::SensorState::kDisconnected, 
        false, 0
    );
}

//! @test Validates timer system handles invalid operations correctly
void test_timer_invalid_operations(void) {
    //! ARRANGE: Prepare invalid timer ID
    const jenlib::time::TimerId invalid_id = jenlib::time::kInvalidTimerId;
    
    //! ACT & ASSERT: Try to cancel invalid timer
    bool cancelled = jenlib::time::Time::cancel_callback(invalid_id);
    TEST_ASSERT_FALSE(cancelled);
    
    //! ACT & ASSERT: Try to schedule with zero delay
    auto timer_id = jenlib::time::Time::schedule_callback(
        0, []() {}, false);  // Zero delay should be invalid
    TEST_ASSERT_EQUAL(jenlib::time::kInvalidTimerId, timer_id);
    
    //! ACT & ASSERT: Try to schedule with null callback
    auto timer_id2 = jenlib::time::Time::schedule_callback(
        100, nullptr, false);  // Null callback should be invalid
    TEST_ASSERT_EQUAL(jenlib::time::kInvalidTimerId, timer_id2);
}

//! @section Main Test Runner

//! @brief Main function that runs all smoke tests
int main() {
    UNITY_BEGIN();
    
    //! State Machine Tests
    RUN_TEST(test_sensor_state_machine_initial_state);
    RUN_TEST(test_sensor_state_machine_connection_transition);
    RUN_TEST(test_sensor_state_machine_session_start_transition);
    RUN_TEST(test_sensor_state_machine_session_end_transition);
    RUN_TEST(test_sensor_state_machine_disconnection_transition);
    
    //! Event System Tests
    RUN_TEST(test_event_system_callback_registration);
    RUN_TEST(test_event_system_event_dispatch_and_processing);
    
    //! Measurement Tests
    RUN_TEST(test_temperature_conversion_accuracy);
    RUN_TEST(test_humidity_conversion_accuracy);
    
    //! Timer System Tests
    RUN_TEST(test_timer_scheduling_and_execution);
    RUN_TEST(test_timer_cancellation);
    
    //! Integration Tests
    RUN_TEST(test_full_sensor_lifecycle_integration);
    
    //! Error Handling Tests
    RUN_TEST(test_state_machine_invalid_transitions);
    RUN_TEST(test_timer_invalid_operations);
    
    return UNITY_END();
}
