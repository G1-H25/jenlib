//! @file smoke_tests/ComprehensiveSmokeTest.cpp
//! @brief Comprehensive smoke test that exercises the full example path
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
#include "jenlib/ble/Ble.h"
#include "jenlib/ble/Messages.h"
#include "jenlib/ble/Ids.h"
#include "jenlib/measurement/Measurement.h"
#include "smoke_tests/PlatformMocks.h"

//! @section Test State Tracking
static std::atomic<int> connection_events{0};
static std::atomic<int> ble_message_events{0};
static std::atomic<int> time_tick_events{0};
static std::atomic<int> measurements_taken{0};
static std::atomic<int> readings_broadcast{0};
static std::atomic<int> receipts_received{0};
static std::vector<jenlib::ble::ReadingMsg> broadcast_readings;
static std::vector<jenlib::ble::ReceiptMsg> received_receipts;

//! @section Test Callbacks

//! @brief Test callback for BLE connection state changes
//! @param connected True if connected, false if disconnected
void test_callback_connection(bool connected) {
    connection_events++;

    //! Dispatch connection state change event
    jenlib::events::Event event(jenlib::events::EventType::kConnectionStateChange,
                               jenlib::time::Time::now(),
                               connected ? 1 : 0);
    jenlib::events::EventDispatcher::dispatch_event(event);
}

//! @brief Test callback for BLE start broadcast messages
//! @param sender_id The device ID of the sender
//! @param msg The start broadcast message
void test_callback_start(jenlib::ble::DeviceId sender_id, const jenlib::ble::StartBroadcastMsg &msg) {
    //! Dispatch BLE message event
    jenlib::events::Event event(jenlib::events::EventType::kBleMessage,
                               jenlib::time::Time::now(),
                               static_cast<std::uint32_t>(jenlib::ble::MessageType::StartBroadcast));
    jenlib::events::EventDispatcher::dispatch_event(event);
}

//! @brief Test callback for BLE receipt messages
//! @param sender_id The device ID of the sender
//! @param msg The receipt message
void test_callback_receipt(jenlib::ble::DeviceId sender_id, const jenlib::ble::ReceiptMsg &msg) {
    receipts_received++;
    received_receipts.push_back(msg);

    //! Dispatch BLE message event
    jenlib::events::Event event(jenlib::events::EventType::kBleMessage,
                               jenlib::time::Time::now(),
                               static_cast<std::uint32_t>(jenlib::ble::MessageType::Receipt));
    jenlib::events::EventDispatcher::dispatch_event(event);
}

//! @brief Test callback for generic BLE messages
//! @param sender_id The device ID of the sender
//! @param payload The BLE payload
void test_callback_generic(jenlib::ble::DeviceId sender_id, const jenlib::ble::BlePayload &payload) {
    //! Dispatch generic BLE message event
    jenlib::events::Event event(jenlib::events::EventType::kBleMessage,
                               jenlib::time::Time::now(),
                               static_cast<std::uint32_t>(jenlib::ble::MessageType::Reading));
    jenlib::events::EventDispatcher::dispatch_event(event);
}

//! @section Event Handlers

//! @brief Test event handler for measurement timer events
void test_handle_measurement_timer() {
    measurements_taken++;

    //! Simulate taking a measurement and broadcasting it
    float temperature_c = 22.5f; // Mock temperature
    float humidity_pct = 45.0f;  // Mock humidity

    jenlib::ble::ReadingMsg reading_msg{
        .sender_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234),
        .offset_ms = jenlib::time::Time::now(),
        .temperature_c_centi = measurement::temperature_to_centi(temperature_c),
        .humidity_bp = measurement::humidity_to_basis_points(humidity_pct)
    };

    jenlib::ble::BLE::broadcast_reading(jenlib::ble::DeviceId(0x12345678), reading_msg);
    readings_broadcast++;
    broadcast_readings.push_back(reading_msg);
}

//! @brief Test event handler for time tick events
//! @param event The time tick event that triggered the handler
void test_handle_time_tick_event(const jenlib::events::Event& event) {
    time_tick_events++;
    test_handle_measurement_timer();
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
    readings_broadcast = 0;
    receipts_received = 0;
    broadcast_readings.clear();
    received_receipts.clear();

    //! ARRANGE: Initialize time service with native driver
    static jenlib::time::NativeTimeDriver native_time_driver;
    jenlib::time::Time::setDriver(&native_time_driver);
    jenlib::time::Time::initialize();
    jenlib::time::Time::clear_all_timers();

    //! ARRANGE: Initialize event dispatcher
    jenlib::events::EventDispatcher::initialize();
    jenlib::events::EventDispatcher::clear_all_callbacks();

    //! ARRANGE: Initialize BLE with mock driver
    static smoke_tests::MockBleDriver mock_ble_driver;
    jenlib::ble::BLE::set_driver(&mock_ble_driver);
    jenlib::ble::BLE::begin();
}

//! @brief Unity test teardown function - cleans up after each test
void tearDown(void) {
    //! CLEANUP: Clean up BLE service and callbacks
    jenlib::ble::BLE::set_connection_callback(nullptr);
    jenlib::ble::BLE::set_start_broadcast_callback(nullptr);
    jenlib::ble::BLE::set_receipt_callback(nullptr);
    jenlib::ble::BLE::set_message_callback(nullptr);
    jenlib::ble::BLE::end();

    //! CLEANUP: Clean up event dispatcher
    jenlib::events::EventDispatcher::clear_all_callbacks();

    //! CLEANUP: Clean up time service
    jenlib::time::Time::clear_all_timers();
}

//! @section Comprehensive Smoke Tests

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

//! @test Validates event-driven connection flow with callbacks
void test_event_driven_connection_flow_with_callbacks(void) {
    //! ARRANGE: Create state machine and register BLE callback only
    jenlib::state::SensorStateMachine sensor_state_machine;
    jenlib::ble::BLE::set_connection_callback(test_callback_connection);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, sensor_state_machine.get_current_state());

    //! ACT: Simulate connection with callback
    test_callback_connection(true);
    jenlib::events::EventDispatcher::process_events();
    sensor_state_machine.handle_connection_change(true);

    //! ASSERT: Verify connection state transition and event processing
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());
    TEST_ASSERT_EQUAL(1, connection_events.load());
}

//! @test Validates event-driven session start flow with callbacks
void test_event_driven_session_start_flow_with_callbacks(void) {
    //! ARRANGE: Create state machine, register handlers, and connect
    jenlib::state::SensorStateMachine sensor_state_machine;
    jenlib::events::EventDispatcher::register_callback(jenlib::events::EventType::kBleMessage, test_handle_ble_message_event);
    jenlib::ble::BLE::set_start_broadcast_callback(test_callback_start);
    sensor_state_machine.handle_connection_change(true);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());

    //! ARRANGE: Prepare start broadcast message
    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };

    //! ACT: Simulate start broadcast with callback
    test_callback_start(jenlib::ble::DeviceId(0x87654321), start_msg);
    jenlib::events::EventDispatcher::process_events();
    bool started = sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);

    //! ASSERT: Verify session started successfully and BLE event processed
    TEST_ASSERT_TRUE(started);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());
    TEST_ASSERT_TRUE(sensor_state_machine.is_session_active());
    TEST_ASSERT_EQUAL(0x1234, sensor_state_machine.get_current_session_id().value());
    TEST_ASSERT_EQUAL(1, ble_message_events.load());
}

//! @test Validates event-driven measurement timer functionality
void test_event_driven_measurement_timer_functionality(void) {
    //! ARRANGE: Register time tick event handler
    jenlib::events::EventDispatcher::register_callback(jenlib::events::EventType::kTimeTick, test_handle_time_tick_event);

    //! ACT: Schedule measurement timer and advance time
    jenlib::time::schedule_repeating_timer(100, test_handle_measurement_timer);
    jenlib::time::Time::delay(150);
    jenlib::time::Time::process_timers();

    //! ASSERT: Verify measurement was taken and broadcast
    TEST_ASSERT_EQUAL(1, measurements_taken.load());
    TEST_ASSERT_EQUAL(1, readings_broadcast.load());
    TEST_ASSERT_EQUAL(1, broadcast_readings.size());
    TEST_ASSERT_EQUAL(0x12345678, broadcast_readings[0].sender_id.value());
    TEST_ASSERT_EQUAL(0x1234, broadcast_readings[0].session_id.value());
}

//! @test Validates event-driven receipt handling functionality
void test_event_driven_receipt_handling_functionality(void) {
    //! ARRANGE: Create state machine, connect, and start session
    jenlib::state::SensorStateMachine sensor_state_machine;
    jenlib::ble::BLE::set_receipt_callback(test_callback_receipt);
    sensor_state_machine.handle_connection_change(true);

    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };
    sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());

    //! ARRANGE: Prepare receipt message
    jenlib::ble::ReceiptMsg receipt_msg{
        .session_id = jenlib::ble::SessionId(0x1234),
        .up_to_offset_ms = 1000
    };

    //! ACT: Simulate receipt with callback
    test_callback_receipt(jenlib::ble::DeviceId(0x87654321), receipt_msg);
    bool receipt_handled = sensor_state_machine.handle_receipt(jenlib::ble::DeviceId(0x87654321), receipt_msg);

    //! ASSERT: Verify receipt was handled successfully
    TEST_ASSERT_TRUE(receipt_handled);
    TEST_ASSERT_EQUAL(1, receipts_received.load());
    TEST_ASSERT_EQUAL(1, received_receipts.size());
}

//! @test Validates event-driven session end functionality
void test_event_driven_session_end_functionality(void) {
    //! ARRANGE: Create state machine, connect, and start session
    jenlib::state::SensorStateMachine sensor_state_machine;
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
}

//! @test Validates event-driven disconnection functionality
void test_event_driven_disconnection_functionality(void) {
    //! ARRANGE: Create state machine, register BLE callback, and connect
    jenlib::state::SensorStateMachine sensor_state_machine;
    jenlib::ble::BLE::set_connection_callback(test_callback_connection);
    sensor_state_machine.handle_connection_change(true);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());

    //! ACT: Simulate disconnection with callback
    test_callback_connection(false);
    jenlib::events::EventDispatcher::process_events();
    sensor_state_machine.handle_connection_change(false);

    //! ASSERT: Verify disconnection processed
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, sensor_state_machine.get_current_state());
    TEST_ASSERT_EQUAL(1, connection_events.load());
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

//! @section Test Runner

//! @brief Main function to run all comprehensive smoke tests
int main() {
    UNITY_BEGIN();

    // State Machine Tests
    RUN_TEST(test_sensor_state_machine_initial_state);
    RUN_TEST(test_sensor_state_machine_connection_transition);
    RUN_TEST(test_sensor_state_machine_session_start);
    RUN_TEST(test_sensor_state_machine_session_end);
    RUN_TEST(test_sensor_state_machine_disconnection_transition);

    // Event-Driven Flow Tests
    RUN_TEST(test_event_driven_connection_flow_with_callbacks);
    RUN_TEST(test_event_driven_session_start_flow_with_callbacks);
    RUN_TEST(test_event_driven_measurement_timer_functionality);
    RUN_TEST(test_event_driven_receipt_handling_functionality);
    RUN_TEST(test_event_driven_session_end_functionality);
    RUN_TEST(test_event_driven_disconnection_functionality);

    // Measurement Conversion Tests
    RUN_TEST(test_temperature_conversion_functionality);
    RUN_TEST(test_humidity_conversion_functionality);

    return UNITY_END();
}

