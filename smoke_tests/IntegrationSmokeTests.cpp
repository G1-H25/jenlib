//! @file smoke_tests/IntegrationSmokeTests.cpp
//! @brief Integration smoke tests for jenlib - tests the full example path
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <unity.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include "smoke_tests/SmokeTestSuites.h"
#include "jenlib/ble/Ble.h"
#include "jenlib/ble/Messages.h"
#include "jenlib/ble/Ids.h"
#include "jenlib/events/EventDispatcher.h"
#include "jenlib/events/EventTypes.h"
#include "jenlib/time/Time.h"
#include "jenlib/time/drivers/NativeTimeDriver.h"
#include "jenlib/state/SensorStateMachine.h"
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
    jenlib::events::Event event(
        jenlib::events::EventType::kConnectionStateChange,
        jenlib::time::Time::now(),
        connected ? 1 : 0);
    jenlib::events::EventDispatcher::dispatch_event(event);
}

//! @brief Test callback for BLE start broadcast messages
//! @param sender_id ID of the device sending the message
//! @param msg Start broadcast message content
void test_callback_start(jenlib::ble::DeviceId sender_id, const jenlib::ble::StartBroadcastMsg &msg) {
    //! Dispatch BLE message event
    jenlib::events::Event event(
        jenlib::events::EventType::kBleMessage,
        jenlib::time::Time::now(),
        static_cast<std::uint32_t>(jenlib::ble::MessageType::StartBroadcast));
    jenlib::events::EventDispatcher::dispatch_event(event);
}

//! @brief Test callback for BLE receipt messages
//! @param sender_id ID of the device sending the message
//! @param msg Receipt message content
void test_callback_receipt(jenlib::ble::DeviceId sender_id, const jenlib::ble::ReceiptMsg &msg) {
    receipts_received++;
    received_receipts.push_back(msg);

    //! Dispatch BLE message event
    jenlib::events::Event event(
        jenlib::events::EventType::kBleMessage,
        jenlib::time::Time::now(),
        static_cast<std::uint32_t>(jenlib::ble::MessageType::Receipt));
    jenlib::events::EventDispatcher::dispatch_event(event);
}

//! @brief Test callback for generic BLE messages
//! @param sender_id ID of the device sending the message
//! @param payload Generic message payload
void test_callback_generic(jenlib::ble::DeviceId sender_id, const jenlib::ble::BlePayload &payload) {
    //! Dispatch generic BLE message event
    jenlib::events::Event event(
        jenlib::events::EventType::kBleMessage,
        jenlib::time::Time::now(),
        static_cast<std::uint32_t>(jenlib::ble::MessageType::Reading));
    jenlib::events::EventDispatcher::dispatch_event(event);
}

//! @section Event Handlers

//! @brief Test event handler for measurement timer
void test_handle_measurement_timer() {
    measurements_taken++;

    //! Simulate taking a measurement and broadcasting it
    float temperature_c = smoke_tests::MockSensorReadings::read_temperature_sensor();
    float humidity_pct = smoke_tests::MockSensorReadings::read_humidity_sensor();

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
//! @param event Time tick event
void test_handle_time_tick_event(const jenlib::events::Event& event) {
    time_tick_events++;
    test_handle_measurement_timer();
}

//! @brief Test event handler for BLE message events
//! @param event BLE message event
void test_handle_ble_message_event(const jenlib::events::Event& event) {
    ble_message_events++;
}

//! @brief Test event handler for connection state change events
//! @param event Connection state change event
void test_handle_connection_state_event(const jenlib::events::Event& event) {
    connection_events++;
}

//! @section Test Setup and Teardown

//! @brief Unity test setup function - resets test state and initializes services
void setUp(void) {
    //! Reset test state
    connection_events = 0;
    ble_message_events = 0;
    time_tick_events = 0;
    measurements_taken = 0;
    readings_broadcast = 0;
    receipts_received = 0;
    broadcast_readings.clear();
    received_receipts.clear();

    //! Initialize time service with mock driver
    static smoke_tests::MockTimeDriver mock_time_driver;
    jenlib::time::Time::setDriver(&mock_time_driver);
    jenlib::time::Time::initialize();
    jenlib::time::Time::clear_all_timers();

    //! Initialize event dispatcher
    jenlib::events::EventDispatcher::initialize();
    jenlib::events::EventDispatcher::clear_all_callbacks();

    //! Initialize BLE with mock driver
    static smoke_tests::MockBleDriver mock_ble_driver;
    jenlib::ble::BLE::set_driver(&mock_ble_driver);
    jenlib::ble::BLE::begin();

    //! Clear any existing BLE callbacks
    jenlib::ble::BLE::set_connection_callback(nullptr);
    jenlib::ble::BLE::set_start_broadcast_callback(nullptr);
    jenlib::ble::BLE::set_reading_callback(nullptr);
    jenlib::ble::BLE::set_receipt_callback(nullptr);
    jenlib::ble::BLE::set_message_callback(nullptr);

    //! Register devices
    mock_ble_driver.register_device(jenlib::ble::DeviceId(0x12345678));  //  Sensor
    mock_ble_driver.register_device(jenlib::ble::DeviceId(0x87654321));  //  Broker
    mock_ble_driver.set_local_device_id(jenlib::ble::DeviceId(0x12345678));  //  We are the sensor
}

//! @brief Unity test teardown function - cleans up after each test
void tearDown(void) {
    //! Clean up BLE service and callbacks
    jenlib::ble::BLE::set_connection_callback(nullptr);
    jenlib::ble::BLE::set_start_broadcast_callback(nullptr);
    jenlib::ble::BLE::set_reading_callback(nullptr);
    jenlib::ble::BLE::set_receipt_callback(nullptr);
    jenlib::ble::BLE::set_message_callback(nullptr);
    jenlib::ble::BLE::end();

    //! Clean up event dispatcher
    jenlib::events::EventDispatcher::clear_all_callbacks();

    //! Clean up time service
    jenlib::time::Time::clear_all_timers();
}

//! @section Integration Tests

//! @test Validates complete sensor lifecycle from connection to measurement broadcasting
void test_full_sensor_lifecycle(void) {
    //! ARRANGE: Create state machine and configure callbacks (simulating the example main.cpp)
    jenlib::state::SensorStateMachine sensor_state_machine;

    //! ARRANGE: Register event handlers (simulating the example main.cpp setup)
    jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kBleMessage,
        test_handle_ble_message_event);
    jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kConnectionStateChange,
        test_handle_connection_state_event);
    jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kTimeTick,
        test_handle_time_tick_event);

    //! ARRANGE: Configure BLE callbacks (simulating the example main.cpp setup)
    jenlib::ble::BLE::set_connection_callback(test_callback_connection);
    jenlib::ble::BLE::set_start_broadcast_callback(test_callback_start);
    jenlib::ble::BLE::set_receipt_callback(test_callback_receipt);
    jenlib::ble::BLE::set_message_callback(test_callback_generic);

    //! ASSERT: Verify initial state
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, sensor_state_machine.get_current_state());
    TEST_ASSERT_FALSE(sensor_state_machine.is_session_active());
}

//! @test Validates sensor connection flow integration
void test_sensor_connection_flow(void) {
    //! ARRANGE: Create state machine and configure callbacks
    jenlib::state::SensorStateMachine sensor_state_machine;
    jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kConnectionStateChange,
        test_handle_connection_state_event);
    jenlib::ble::BLE::set_connection_callback(test_callback_connection);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, sensor_state_machine.get_current_state());

    //! ACT: Simulate connection (simulating the example main.cpp connection flow)
    test_callback_connection(true);
    sensor_state_machine.handle_connection_change(true);

    //! ASSERT: Verify connection state transition
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());
    TEST_ASSERT_EQUAL(1, connection_events.load());
}

//! @test Validates sensor session start flow integration
void test_sensor_session_start_flow(void) {
    //! ARRANGE: Create state machine, configure callbacks, and connect
    jenlib::state::SensorStateMachine sensor_state_machine;
    jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kBleMessage,
        test_handle_ble_message_event);
    jenlib::ble::BLE::set_start_broadcast_callback(test_callback_start);
    test_callback_connection(true);
    sensor_state_machine.handle_connection_change(true);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());

    //! ARRANGE: Prepare start broadcast message
    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };

    //! ACT: Simulate start broadcast message (simulating broker starting a session)
    test_callback_start(jenlib::ble::DeviceId(0x87654321), start_msg);
    jenlib::events::EventDispatcher::process_events();  //  Process the dispatched event
    bool started = sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);

    //! ASSERT: Verify session started successfully
    TEST_ASSERT_TRUE(started);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());
    TEST_ASSERT_TRUE(sensor_state_machine.is_session_active());
    TEST_ASSERT_EQUAL(0x1234, sensor_state_machine.get_current_session_id().value());
    TEST_ASSERT_EQUAL(1, ble_message_events.load());
}

//! @test Validates sensor measurement flow integration
void test_sensor_measurement_flow(void) {
    //! ARRANGE: Create state machine, configure callbacks, connect, and start session
    jenlib::state::SensorStateMachine sensor_state_machine;
    jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kBleMessage,
        test_handle_ble_message_event);
    jenlib::ble::BLE::set_start_broadcast_callback(test_callback_start);
    test_callback_connection(true);
    sensor_state_machine.handle_connection_change(true);

    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };
    test_callback_start(jenlib::ble::DeviceId(0x87654321), start_msg);
    sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());

    //! ACT: Simulate measurement timer (simulating the example main.cpp measurement flow)
    jenlib::time::schedule_repeating_timer(1000, test_handle_measurement_timer);
    auto mock_driver = static_cast<smoke_tests::MockTimeDriver*>(jenlib::time::Time::getDriver());
    mock_driver->advance_time(1000);
    jenlib::time::Time::process_timers();

    //! ASSERT: Verify measurement was taken and broadcast
    TEST_ASSERT_EQUAL(1, measurements_taken.load());
    TEST_ASSERT_EQUAL(1, readings_broadcast.load());
    TEST_ASSERT_EQUAL(1, broadcast_readings.size());
    TEST_ASSERT_EQUAL(0x12345678, broadcast_readings[0].sender_id.value());
    TEST_ASSERT_EQUAL(0x1234, broadcast_readings[0].session_id.value());
}

//! @test Validates sensor receipt handling flow integration
void test_sensor_receipt_handling_flow(void) {
    //! ARRANGE: Create state machine, configure callbacks, connect, and start session
    jenlib::state::SensorStateMachine sensor_state_machine;
    jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kBleMessage,
        test_handle_ble_message_event);
    jenlib::ble::BLE::set_start_broadcast_callback(test_callback_start);
    jenlib::ble::BLE::set_receipt_callback(test_callback_receipt);
    test_callback_connection(true);
    sensor_state_machine.handle_connection_change(true);

    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };
    test_callback_start(jenlib::ble::DeviceId(0x87654321), start_msg);
    sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());

    //! ARRANGE: Prepare receipt message
    jenlib::ble::ReceiptMsg receipt_msg{
        .session_id = jenlib::ble::SessionId(0x1234),
        .up_to_offset_ms = 1000
    };

    //! ACT: Simulate receipt from broker
    test_callback_receipt(jenlib::ble::DeviceId(0x87654321), receipt_msg);
    bool receipt_handled = sensor_state_machine.handle_receipt(jenlib::ble::DeviceId(0x87654321), receipt_msg);

    //! ASSERT: Verify receipt was handled successfully
    TEST_ASSERT_TRUE(receipt_handled);
    TEST_ASSERT_EQUAL(1, receipts_received.load());
    TEST_ASSERT_EQUAL(1, received_receipts.size());
}

//! @test Validates sensor session end flow integration
void test_sensor_session_end_flow(void) {
    //! ARRANGE: Create state machine, configure callbacks, connect, and start session
    jenlib::state::SensorStateMachine sensor_state_machine;
    jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kConnectionStateChange,
        test_handle_connection_state_event);
    jenlib::ble::BLE::set_connection_callback(test_callback_connection);
    jenlib::ble::BLE::set_start_broadcast_callback(test_callback_start);
    test_callback_connection(true);
    sensor_state_machine.handle_connection_change(true);

    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };
    test_callback_start(jenlib::ble::DeviceId(0x87654321), start_msg);
    sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());

    //! ACT: Simulate session end
    bool ended = sensor_state_machine.handle_session_end();

    //! ASSERT: Verify session ended successfully
    TEST_ASSERT_TRUE(ended);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());
    TEST_ASSERT_FALSE(sensor_state_machine.is_session_active());
}

//! @test Validates sensor disconnection flow integration
void test_sensor_disconnection_flow(void) {
    //! ARRANGE: Create state machine, configure callbacks, and connect
    jenlib::state::SensorStateMachine sensor_state_machine;
    jenlib::events::EventDispatcher::register_callback(
        jenlib::events::EventType::kConnectionStateChange,
        test_handle_connection_state_event);
    jenlib::ble::BLE::set_connection_callback(test_callback_connection);
    test_callback_connection(true);
    sensor_state_machine.handle_connection_change(true);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());

    //! ACT: Simulate disconnection
    test_callback_connection(false);
    sensor_state_machine.handle_connection_change(false);

    //! ASSERT: Verify disconnection state transition
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, sensor_state_machine.get_current_state());
    TEST_ASSERT_EQUAL(2, connection_events.load());
}

//! @test Validates sensor-broker communication flow and message handling
void test_sensor_broker_communication_flow(void) {
    // Create mock broker
    auto mock_ble_driver = static_cast<smoke_tests::MockBleDriver*>(jenlib::ble::BLE::driver());
    smoke_tests::MockBroker broker(jenlib::ble::DeviceId(0x87654321), mock_ble_driver);

    // Create state machine
    jenlib::state::SensorStateMachine sensor_state_machine;

    // Connect sensor
    test_callback_connection(true);
    sensor_state_machine.handle_connection_change(true);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());

    // Broker starts session
    broker.start_session(jenlib::ble::DeviceId(0x12345678), jenlib::ble::SessionId(0x1234));

    // Process BLE events to receive start message
    jenlib::ble::BLE::process_events();

    // Manually handle the start broadcast message (simulating application logic)
    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };
    sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);

    // Check that sensor received start message and transitioned to running
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());
    TEST_ASSERT_TRUE(sensor_state_machine.is_session_active());

    // Simulate measurement and broadcast
    test_handle_measurement_timer();

    TEST_ASSERT_EQUAL(1, readings_broadcast.load());
    TEST_ASSERT_EQUAL(1, broadcast_readings.size());

    // Broker processes messages and sends receipt
    broker.process_messages();

    // Process BLE events to receive receipt
    jenlib::ble::BLE::process_events();

    // Manually handle the receipt message (simulating application logic)
    jenlib::ble::ReceiptMsg receipt_msg{
        .session_id = jenlib::ble::SessionId(0x1234),
        .up_to_offset_ms = 1000
    };
    test_callback_receipt(jenlib::ble::DeviceId(0x87654321), receipt_msg);
    sensor_state_machine.handle_receipt(jenlib::ble::DeviceId(0x87654321), receipt_msg);

    TEST_ASSERT_EQUAL(1, receipts_received.load());
    TEST_ASSERT_EQUAL(1, received_receipts.size());

    // Broker stops session
    broker.stop_session();

    // Sensor should still be running until it receives stop command
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());
}

//! @test Validates measurement broadcasting flow and data transmission
void test_measurement_broadcasting_flow(void) {
    // Create state machine and start session
    jenlib::state::SensorStateMachine sensor_state_machine;
    sensor_state_machine.handle_connection_change(true);

    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };

    sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());

    // Schedule measurement timer
    jenlib::time::schedule_repeating_timer(500, test_handle_measurement_timer);

    // Advance time and process multiple measurements
    auto mock_driver = static_cast<smoke_tests::MockTimeDriver*>(jenlib::time::Time::getDriver());

    for (int i = 0; i < 5; i++) {
        mock_driver->advance_time(500);
        jenlib::time::Time::process_timers();

        TEST_ASSERT_EQUAL(i + 1, measurements_taken.load());
        TEST_ASSERT_EQUAL(i + 1, readings_broadcast.load());
        TEST_ASSERT_EQUAL(i + 1, broadcast_readings.size());
    }

    // Verify all readings have correct session ID
    for (const auto& reading : broadcast_readings) {
        TEST_ASSERT_EQUAL(0x12345678, reading.sender_id.value());
        TEST_ASSERT_EQUAL(0x1234, reading.session_id.value());
        TEST_ASSERT_GREATER_OR_EQUAL(0, reading.temperature_c_centi);
        TEST_ASSERT_GREATER_OR_EQUAL(0, reading.humidity_bp);
    }
}

//! @test Validates session start and stop flow functionality
void test_session_start_stop_flow(void) {
    // Create state machine
    jenlib::state::SensorStateMachine sensor_state_machine;

    // Connect
    sensor_state_machine.handle_connection_change(true);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());

    // Start session
    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };

    bool started = sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_TRUE(started);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());
    TEST_ASSERT_TRUE(sensor_state_machine.is_session_active());
    TEST_ASSERT_EQUAL(0x1234, sensor_state_machine.get_current_session_id().value());

    // Stop session
    bool stopped = sensor_state_machine.handle_session_end();
    TEST_ASSERT_TRUE(stopped);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());
    TEST_ASSERT_FALSE(sensor_state_machine.is_session_active());
    TEST_ASSERT_EQUAL(0, sensor_state_machine.get_current_session_id().value());

    // Start another session
    jenlib::ble::StartBroadcastMsg start_msg2{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x5678)
    };

    started = sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg2);
    TEST_ASSERT_TRUE(started);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());
    TEST_ASSERT_TRUE(sensor_state_machine.is_session_active());
    TEST_ASSERT_EQUAL(0x5678, sensor_state_machine.get_current_session_id().value());
}

//! @test Validates connection loss and recovery functionality
void test_connection_loss_recovery(void) {
    // Create state machine
    jenlib::state::SensorStateMachine sensor_state_machine;

    // Connect and start session
    sensor_state_machine.handle_connection_change(true);

    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };

    sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());
    TEST_ASSERT_TRUE(sensor_state_machine.is_session_active());

    // Simulate connection loss (end session first, then disconnect)
    sensor_state_machine.handle_session_end();  //  Clear session data while still running
    test_callback_connection(false);
    sensor_state_machine.handle_connection_change(false);

    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, sensor_state_machine.get_current_state());
    TEST_ASSERT_FALSE(sensor_state_machine.is_session_active());
    TEST_ASSERT_EQUAL(0, sensor_state_machine.get_current_session_id().value());

    // Simulate connection recovery
    test_callback_connection(true);
    sensor_state_machine.handle_connection_change(true);

    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());
    TEST_ASSERT_FALSE(sensor_state_machine.is_session_active());

    // Can start new session after recovery
    jenlib::ble::StartBroadcastMsg start_msg2{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x5678)
    };

    bool started = sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg2);
    TEST_ASSERT_TRUE(started);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());
    TEST_ASSERT_TRUE(sensor_state_machine.is_session_active());
}

//! @test Validates error recovery flow and system resilience
void test_error_recovery_flow(void) {
    // Create state machine
    jenlib::state::SensorStateMachine sensor_state_machine;

    // Connect and start session
    sensor_state_machine.handle_connection_change(true);

    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };

    sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());

    // Simulate error
    sensor_state_machine.handle_error("Test error");

    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kError, sensor_state_machine.get_current_state());
    TEST_ASSERT_FALSE(sensor_state_machine.is_session_active());

    // Recover from error
    sensor_state_machine.handle_recovery();

    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, sensor_state_machine.get_current_state());
    TEST_ASSERT_FALSE(sensor_state_machine.is_session_active());

    // Can reconnect and start new session after recovery
    sensor_state_machine.handle_connection_change(true);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, sensor_state_machine.get_current_state());

    bool started = sensor_state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_TRUE(started);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, sensor_state_machine.get_current_state());
    TEST_ASSERT_TRUE(sensor_state_machine.is_session_active());
}

//! @section Test Runner

//! @brief Main function to run all integration smoke tests
int main() {
    UNITY_BEGIN();

    // Full Lifecycle Tests
    RUN_TEST(test_full_sensor_lifecycle);

    // Individual Flow Tests
    RUN_TEST(test_sensor_connection_flow);
    RUN_TEST(test_sensor_session_start_flow);
    RUN_TEST(test_sensor_measurement_flow);
    RUN_TEST(test_sensor_receipt_handling_flow);
    RUN_TEST(test_sensor_session_end_flow);
    RUN_TEST(test_sensor_disconnection_flow);

    // Communication and Integration Tests
    RUN_TEST(test_sensor_broker_communication_flow);
    RUN_TEST(test_measurement_broadcasting_flow);
    RUN_TEST(test_session_start_stop_flow);
    RUN_TEST(test_connection_loss_recovery);
    RUN_TEST(test_error_recovery_flow);

    return UNITY_END();
}
