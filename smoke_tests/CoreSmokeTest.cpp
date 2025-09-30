//! @file smoke_tests/CoreSmokeTest.cpp
//! @brief Core smoke test that exercises the event system and state machine
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <unity.h>
#include <atomic>
#include <vector>
#include "jenlib/events/EventDispatcher.h"
#include "jenlib/events/EventTypes.h"
#include "jenlib/time/Time.h"
#include "jenlib/time/drivers/NativeTimeDriver.h"
#include "jenlib/state/SensorStateMachine.h"
#include "jenlib/measurement/Measurement.h"

//! @section Test State Tracking
static std::atomic<int> connection_events{0};
static std::atomic<int> ble_message_events{0};
static std::atomic<int> time_tick_events{0};
static std::atomic<int> measurements_taken{0};

//! @section Event Handlers

//! @brief Test event handler for time tick events
//! @param event The time tick event that triggered the handler
void test_handle_time_tick_event(const jenlib::events::Event& event) {
    time_tick_events++;
    measurements_taken++;
}

//! @brief Test event handler for BLE message events
//! @param event The BLE message event that triggered the handler
void test_handle_ble_message_event(const jenlib::events::Event& event) {
    ble_message_events++;
}

//! @brief Test event handler for connection state change events
//! @param event The connection state change event that triggered the handler
void test_handle_connection_state_event(const jenlib::events::Event& event) {
    connection_events++;
}

//! @section Test Setup and Teardown

//! @brief Unity test setup function - resets test state and initializes services
void setUp(void) {
    //! ARRANGE: Reset test state
    connection_events = 0;
    ble_message_events = 0;
    time_tick_events = 0;
    measurements_taken = 0;

    //! ARRANGE: Initialize time service with native driver
    static jenlib::time::NativeTimeDriver native_time_driver;
    jenlib::time::Time::setDriver(&native_time_driver);
    jenlib::time::Time::initialize();
    jenlib::time::Time::clear_all_timers();

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

//! @section Core Smoke Tests

//! @test Validates sensor state machine initial state
void test_sensor_state_machine_initial_state(void) {
    //! ARRANGE: Create state machine
    jenlib::state::SensorStateMachine sensor_state_machine;

    //! ACT: No action needed - testing initial state

    //! ASSERT: Verify initial state
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, sensor_state_machine.get_current_state());
    TEST_ASSERT_FALSE(sensor_state_machine.is_session_active());
    TEST_ASSERT_EQUAL(0, sensor_state_machine.get_current_session_id().value());
}

//! @test Validates sensor state machine connection transition
void test_sensor_state_machine_connection_transition(void) {
    //! ARRANGE: Create state machine
    jenlib::state::SensorStateMachine sensor_state_machine;
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, sensor_state_machine.get_current_state());

    //! ACT: Handle connection change
    bool transitioned = sensor_state_machine.handle_connection_change(true);

    //! ASSERT: Verify transition to waiting state
    TEST_ASSERT_TRUE(transitioned);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());
    TEST_ASSERT_TRUE(sensor_state_machine.is_in_state(jenlib::state::SensorState::kWaiting));
}

//! @test Validates sensor state machine session start functionality
void test_sensor_state_machine_session_start(void) {
    //! ARRANGE: Create state machine and connect
    jenlib::state::SensorStateMachine sensor_state_machine;
    sensor_state_machine.handle_connection_change(true);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());

    //! ARRANGE: Prepare start broadcast message
    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };

    //! ACT: Handle start broadcast message
    bool started = sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);

    //! ASSERT: Verify session started successfully
    TEST_ASSERT_TRUE(started);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());
    TEST_ASSERT_TRUE(sensor_state_machine.is_in_state(jenlib::state::SensorState::kRunning));
    TEST_ASSERT_TRUE(sensor_state_machine.is_session_active());
    TEST_ASSERT_EQUAL(0x1234, sensor_state_machine.get_current_session_id().value());
}

//! @test Validates sensor state machine session end functionality
void test_sensor_state_machine_session_end(void) {
    //! ARRANGE: Create state machine, connect, and start session
    jenlib::state::SensorStateMachine sensor_state_machine;
    sensor_state_machine.handle_connection_change(true);

    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };
    sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());

    //! ACT: Handle session end
    bool ended = sensor_state_machine.handle_session_end();

    //! ASSERT: Verify session ended successfully
    TEST_ASSERT_TRUE(ended);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());
    TEST_ASSERT_TRUE(sensor_state_machine.is_in_state(jenlib::state::SensorState::kWaiting));
    TEST_ASSERT_FALSE(sensor_state_machine.is_session_active());
    TEST_ASSERT_EQUAL(0, sensor_state_machine.get_current_session_id().value());
}

//! @test Validates sensor state machine disconnection transition
void test_sensor_state_machine_disconnection_transition(void) {
    //! ARRANGE: Create state machine and connect
    jenlib::state::SensorStateMachine sensor_state_machine;
    sensor_state_machine.handle_connection_change(true);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());

    //! ACT: Handle disconnection
    bool transitioned = sensor_state_machine.handle_connection_change(false);

    //! ASSERT: Verify transition back to disconnected state
    TEST_ASSERT_TRUE(transitioned);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, sensor_state_machine.get_current_state());
    TEST_ASSERT_TRUE(sensor_state_machine.is_in_state(jenlib::state::SensorState::kDisconnected));
}

//! @test Validates event-driven connection flow
void test_event_driven_connection_flow(void) {
    //! ARRANGE: Create state machine and register event handlers
    jenlib::state::SensorStateMachine sensor_state_machine;
    jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kConnectionStateChange,
        test_handle_connection_state_event);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, sensor_state_machine.get_current_state());

    //! ACT: Simulate connection event and handle connection change
    jenlib::events::Event connection_event(
        jenlib::events::EventType::kConnectionStateChange,
        jenlib::time::Time::now(),
        1);
    jenlib::events::EventDispatcher::dispatch_event(connection_event);
    jenlib::events::EventDispatcher::process_events();
    sensor_state_machine.handle_connection_change(true);

    //! ASSERT: Verify connection state transition and event processing
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());
    TEST_ASSERT_EQUAL(1, connection_events.load());
}

//! @test Validates event-driven session start flow
void test_event_driven_session_start_flow(void) {
    //! ARRANGE: Create state machine, register handlers, and connect
    jenlib::state::SensorStateMachine sensor_state_machine;
    jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kBleMessage,
        test_handle_ble_message_event);
    sensor_state_machine.handle_connection_change(true);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());

    //! ARRANGE: Prepare start broadcast message
    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };

    //! ACT: Simulate BLE message event and handle start broadcast
    jenlib::events::Event ble_event(
        jenlib::events::EventType::kBleMessage,
        jenlib::time::Time::now(),
        static_cast<std::uint32_t>(jenlib::ble::MessageType::StartBroadcast));
    jenlib::events::EventDispatcher::dispatch_event(ble_event);
    jenlib::events::EventDispatcher::process_events();
    bool started = sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);

    //! ASSERT: Verify session started successfully and BLE event processed
    TEST_ASSERT_TRUE(started);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());
    TEST_ASSERT_TRUE(sensor_state_machine.is_session_active());
    TEST_ASSERT_EQUAL(0x1234, sensor_state_machine.get_current_session_id().value());
    TEST_ASSERT_EQUAL(1, ble_message_events.load());
}

//! @test Validates event-driven time tick processing
void test_event_driven_time_tick_processing(void) {
    //! ARRANGE: Register time tick event handler
    jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick,
        test_handle_time_tick_event);
    TEST_ASSERT_EQUAL(1, jenlib::events::EventDispatcher::get_callback_count(jenlib::events::EventType::kTimeTick));

    //! ACT: Dispatch and process time tick event
    jenlib::events::Event time_event(
        jenlib::events::EventType::kTimeTick,
        jenlib::time::Time::now(),
        0);
    auto enqueue_result = jenlib::events::EventDispatcher::dispatch_event(time_event);
    auto processed_count = jenlib::events::EventDispatcher::process_events();

    //! ASSERT: Verify event was dispatched and processed correctly
    TEST_ASSERT_EQUAL(static_cast<int>(jenlib::events::EventEnqueueResult::Enqueued),
                      static_cast<int>(enqueue_result));
    TEST_ASSERT_EQUAL(1, processed_count);
    TEST_ASSERT_EQUAL(1, time_tick_events.load());
    TEST_ASSERT_EQUAL(1, measurements_taken.load());
}

//! @test Validates event-driven session end functionality
void test_event_driven_session_end(void) {
    //! ARRANGE: Create state machine, register handlers, connect, and start session
    jenlib::state::SensorStateMachine sensor_state_machine;
    jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kConnectionStateChange,
        test_handle_connection_state_event);

    jenlib::events::Event connect_event(
        jenlib::events::EventType::kConnectionStateChange,
        jenlib::time::Time::now(),
        1);
    jenlib::events::EventDispatcher::dispatch_event(connect_event);
    jenlib::events::EventDispatcher::process_events();
    sensor_state_machine.handle_connection_change(true);

    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };
    sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());

    //! ACT: End session
    bool ended = sensor_state_machine.handle_session_end();

    //! ASSERT: Verify session ended successfully
    TEST_ASSERT_TRUE(ended);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());
    TEST_ASSERT_FALSE(sensor_state_machine.is_session_active());
    TEST_ASSERT_EQUAL(1, connection_events.load());  //  Only connection event processed
}

//! @test Validates event-driven disconnection functionality
void test_event_driven_disconnection(void) {
    //! ARRANGE: Create state machine and register handlers
    jenlib::state::SensorStateMachine sensor_state_machine;
    jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kConnectionStateChange,
        test_handle_connection_state_event);

    //! ARRANGE: Connect and start session
    jenlib::events::Event connect_event(
        jenlib::events::EventType::kConnectionStateChange,
        jenlib::time::Time::now(),
        1);
    jenlib::events::EventDispatcher::dispatch_event(connect_event);
    jenlib::events::EventDispatcher::process_events();
    sensor_state_machine.handle_connection_change(true);

    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };
    sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);

    //! ACT: End session and simulate disconnection
    sensor_state_machine.handle_session_end();
    jenlib::events::Event disconnect_event(
        jenlib::events::EventType::kConnectionStateChange,
        jenlib::time::Time::now(),
        0);
    jenlib::events::EventDispatcher::dispatch_event(disconnect_event);
    jenlib::events::EventDispatcher::process_events();
    sensor_state_machine.handle_connection_change(false);

    //! ASSERT: Verify disconnection processed
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, sensor_state_machine.get_current_state());
    TEST_ASSERT_EQUAL(2, connection_events.load());  //  Both connection and disconnection events processed
}

//! @test Validates temperature conversion functionality
void test_temperature_conversion_functionality(void) {
    //! ARRANGE: Prepare test temperature values
    float temp_c = 22.5f;
    float negative_temp_c = -10.0f;

    //! ACT: Convert temperatures to centi-degrees
    std::int16_t temp_centi = measurement::temperature_to_centi(temp_c);
    std::int16_t negative_temp_centi = measurement::temperature_to_centi(negative_temp_c);

    //! ASSERT: Verify temperature conversions are correct
    TEST_ASSERT_EQUAL(2250, temp_centi);
    TEST_ASSERT_EQUAL(-1000, negative_temp_centi);
}

//! @test Validates humidity conversion functionality
void test_humidity_conversion_functionality(void) {
    //! ARRANGE: Prepare test humidity values
    float humidity_pct = 45.0f;
    float max_humidity_pct = 100.0f;

    //! ACT: Convert humidity percentages to basis points
    std::uint16_t humidity_bp = measurement::humidity_to_basis_points(humidity_pct);
    std::uint16_t max_humidity_bp = measurement::humidity_to_basis_points(max_humidity_pct);

    //! ASSERT: Verify humidity conversions are correct
    TEST_ASSERT_EQUAL(4500, humidity_bp);
    TEST_ASSERT_EQUAL(10000, max_humidity_bp);
}

//! @test Validates event system multi-type callback registration
void test_event_system_multi_type_callback_registration(void) {
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

    //! ASSERT: Verify all callbacks were registered successfully
    TEST_ASSERT_EQUAL(3, jenlib::events::EventDispatcher::get_total_callback_count());
}

//! @test Validates event system multi-type event processing
void test_event_system_multi_type_event_processing(void) {
    //! ARRANGE: Register callbacks for different event types
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
    TEST_ASSERT_EQUAL(3, event_count.load());

    //! CLEANUP: Unregister callbacks
    jenlib::events::EventDispatcher::unregister_callback(time_id);
    jenlib::events::EventDispatcher::unregister_callback(connection_id);
    jenlib::events::EventDispatcher::unregister_callback(ble_id);
}

//! @section Test Runner

//! @brief Main function to run all core smoke tests
int main() {
    UNITY_BEGIN();

    // State Machine Tests
    RUN_TEST(test_sensor_state_machine_initial_state);
    RUN_TEST(test_sensor_state_machine_connection_transition);
    RUN_TEST(test_sensor_state_machine_session_start);
    RUN_TEST(test_sensor_state_machine_session_end);
    RUN_TEST(test_sensor_state_machine_disconnection_transition);

    // Event-Driven Flow Tests
    RUN_TEST(test_event_driven_connection_flow);
    RUN_TEST(test_event_driven_session_start_flow);
    RUN_TEST(test_event_driven_time_tick_processing);
    RUN_TEST(test_event_driven_session_end);
    RUN_TEST(test_event_driven_disconnection);

    // Measurement Conversion Tests
    RUN_TEST(test_temperature_conversion_functionality);
    RUN_TEST(test_humidity_conversion_functionality);

    // Event System Integration Tests
    RUN_TEST(test_event_system_multi_type_callback_registration);
    RUN_TEST(test_event_system_multi_type_event_processing);

    return UNITY_END();
}

