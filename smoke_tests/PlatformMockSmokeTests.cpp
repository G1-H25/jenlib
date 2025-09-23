//! @file smoke_tests/PlatformMockSmokeTests.cpp
//! @brief Platform mock smoke tests for jenlib
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include "unity.h"
#include "SmokeTestSuites.h"
#include <smoke_tests/PlatformMocks.h>
#include <jenlib/time/Time.h>
#include <jenlib/ble/BLE.h>
#include <jenlib/ble/Messages.h>
#include <jenlib/ble/Ids.h>
#include <atomic>
#include <thread>
#include <chrono>

//! @section Test State Tracking
static std::atomic<int> mock_time_calls{0};
static std::atomic<int> mock_ble_calls{0};
static std::atomic<int> mock_sensor_calls{0};

//! @section Test Setup and Teardown

//! @brief Unity test setup function - resets mock call counters
void setUp(void) {
    //! Reset test state
    mock_time_calls = 0;
    mock_ble_calls = 0;
    mock_sensor_calls = 0;
}

//! @brief Unity test teardown function - cleans up after each test
void tearDown(void) {
    //! Clean up
}

//! @section Mock Time Driver Tests

//! @test Validates mock time driver initial state
void test_mock_time_driver_initial_state(void) {
    //! ARRANGE: Create mock time driver
    smoke_tests::MockTimeDriver mock_driver;
    
    //! ACT: No action needed - testing initial state
    
    //! ASSERT: Verify initial state
    TEST_ASSERT_EQUAL(0, mock_driver.now());
}

//! @test Validates mock time driver time advancement functionality
void test_mock_time_driver_time_advancement(void) {
    //! ARRANGE: Create mock time driver
    smoke_tests::MockTimeDriver mock_driver;
    
    //! ACT: Advance time
    mock_driver.advance_time(1000);
    
    //! ASSERT: Verify time advanced correctly
    TEST_ASSERT_EQUAL(1000, mock_driver.now());
    
    //! ACT: Advance time again
    mock_driver.advance_time(500);
    
    //! ASSERT: Verify cumulative time advancement
    TEST_ASSERT_EQUAL(1500, mock_driver.now());
}

//! @test Validates mock time driver time setting functionality
void test_mock_time_driver_time_setting(void) {
    //! ARRANGE: Create mock time driver and advance time
    smoke_tests::MockTimeDriver mock_driver;
    mock_driver.advance_time(1000);
    TEST_ASSERT_EQUAL(1000, mock_driver.now());
    
    //! ACT: Set time to specific value
    mock_driver.set_time(2000);
    
    //! ASSERT: Verify time was set correctly
    TEST_ASSERT_EQUAL(2000, mock_driver.now());
}

//! @test Validates mock time driver delay functionality
void test_mock_time_driver_delay(void) {
    //! ARRANGE: Create mock time driver and set initial time
    smoke_tests::MockTimeDriver mock_driver;
    mock_driver.set_time(2000);
    TEST_ASSERT_EQUAL(2000, mock_driver.now());
    
    //! ACT: Execute delay
    mock_driver.delay(1000);
    
    //! ASSERT: Verify delay advanced time correctly
    TEST_ASSERT_EQUAL(3000, mock_driver.now());
}

//! @test Validates mock time driver reset functionality
void test_mock_time_driver_reset(void) {
    //! ARRANGE: Create mock time driver and advance time
    smoke_tests::MockTimeDriver mock_driver;
    mock_driver.advance_time(5000);
    TEST_ASSERT_EQUAL(5000, mock_driver.now());
    
    //! ACT: Reset time driver
    mock_driver.reset();
    
    //! ASSERT: Verify time was reset to zero
    TEST_ASSERT_EQUAL(0, mock_driver.now());
}

//! @test Validates mock time driver integration with Time service
void test_mock_time_driver_time_service_integration(void) {
    //! ARRANGE: Create mock time driver and integrate with Time service
    smoke_tests::MockTimeDriver mock_driver;
    jenlib::time::Time::setDriver(&mock_driver);
    jenlib::time::Time::initialize();
    
    //! ACT: Advance time using mock driver
    mock_driver.advance_time(5000);
    
    //! ASSERT: Verify Time service reflects mock driver time
    TEST_ASSERT_EQUAL(5000, jenlib::time::Time::now());
}

//! @test Validates mock time driver timer scheduling and execution
void test_mock_time_driver_timer_scheduling(void) {
    //! ARRANGE: Create mock time driver and integrate with Time service
    smoke_tests::MockTimeDriver mock_driver;
    jenlib::time::Time::setDriver(&mock_driver);
    jenlib::time::Time::initialize();
    
    std::atomic<bool> timer_fired{false};
    auto timer_id = jenlib::time::Time::schedule_callback(1000, [&timer_fired]() {
        timer_fired = true;
    }, false);
    
    //! ASSERT: Verify timer was scheduled correctly
    TEST_ASSERT_NOT_EQUAL(jenlib::time::kInvalidTimerId, timer_id);
    TEST_ASSERT_EQUAL(1, jenlib::time::Time::get_active_timer_count());
    
    //! ACT: Advance time to trigger timer
    mock_driver.advance_time(1000);
    auto fired_count = jenlib::time::Time::process_timers();
    
    //! ASSERT: Verify timer fired correctly and is now inactive
    TEST_ASSERT_EQUAL(1, fired_count);
    TEST_ASSERT_TRUE(timer_fired.load());
    TEST_ASSERT_EQUAL(0, jenlib::time::Time::get_active_timer_count());
}

//! @section Mock BLE Driver Tests

//! @test Validates mock BLE driver initial state
void test_mock_ble_driver_initial_state(void) {
    //! ARRANGE: Create mock BLE driver
    smoke_tests::MockBleDriver mock_driver;
    
    //! ACT: No action needed - testing initial state
    
    //! ASSERT: Verify initial state
    TEST_ASSERT_FALSE(mock_driver.is_connected());
    TEST_ASSERT_EQUAL(0, mock_driver.get_local_device_id().value());
}

//! @test Validates mock BLE driver initialization
void test_mock_ble_driver_initialization(void) {
    //! ARRANGE: Create mock BLE driver
    smoke_tests::MockBleDriver mock_driver;
    TEST_ASSERT_FALSE(mock_driver.is_connected());
    
    //! ACT: Initialize driver
    bool initialized = mock_driver.begin();
    
    //! ASSERT: Verify initialization
    TEST_ASSERT_TRUE(initialized);
    TEST_ASSERT_TRUE(mock_driver.is_connected());
}

//! @test Validates mock BLE driver device registration
void test_mock_ble_driver_device_registration(void) {
    //! ARRANGE: Create and initialize mock BLE driver
    smoke_tests::MockBleDriver mock_driver;
    mock_driver.begin();
    
    const jenlib::ble::DeviceId sensor_id = jenlib::ble::DeviceId(0x12345678);
    const jenlib::ble::DeviceId broker_id = jenlib::ble::DeviceId(0x87654321);
    
    //! ACT: Register devices and set local device
    mock_driver.register_device(sensor_id);
    mock_driver.register_device(broker_id);
    mock_driver.set_local_device_id(sensor_id);
    
    //! ASSERT: Verify device registration
    TEST_ASSERT_EQUAL(sensor_id.value(), mock_driver.get_local_device_id().value());
}

//! @test Validates mock BLE driver point-to-point messaging
void test_mock_ble_driver_point_to_point_messaging(void) {
    //! ARRANGE: Set up mock BLE driver with devices
    smoke_tests::MockBleDriver mock_driver;
    mock_driver.begin();
    
    const jenlib::ble::DeviceId sensor_id = jenlib::ble::DeviceId(0x12345678);
    const jenlib::ble::DeviceId broker_id = jenlib::ble::DeviceId(0x87654321);
    
    mock_driver.register_device(sensor_id);
    mock_driver.register_device(broker_id);
    mock_driver.set_local_device_id(sensor_id);
    
    jenlib::ble::BlePayload test_payload;
    test_payload.append_u8(0x01);
    test_payload.append_u8(0x02);
    test_payload.append_u8(0x03);
    
    //! ACT: Send message
    mock_driver.send_to(broker_id, std::move(test_payload));
    
    //! ASSERT: Verify message was queued
    TEST_ASSERT_EQUAL(1, mock_driver.get_message_count(broker_id));
    
    //! ACT: Receive message
    jenlib::ble::BlePayload received_payload;
    bool received = mock_driver.receive(broker_id, received_payload);
    
    //! ASSERT: Verify message content
    TEST_ASSERT_TRUE(received);
    TEST_ASSERT_EQUAL(3, static_cast<int>(received_payload.size));
    TEST_ASSERT_EQUAL(0x01, received_payload.bytes[0]);
    TEST_ASSERT_EQUAL(0x02, received_payload.bytes[1]);
    TEST_ASSERT_EQUAL(0x03, received_payload.bytes[2]);
}

//! @test Validates mock BLE driver broadcast messaging
void test_mock_ble_driver_broadcast_messaging(void) {
    //! ARRANGE: Set up mock BLE driver with devices
    smoke_tests::MockBleDriver mock_driver;
    mock_driver.begin();
    
    const jenlib::ble::DeviceId sensor_id = jenlib::ble::DeviceId(0x12345678);
    const jenlib::ble::DeviceId broker_id = jenlib::ble::DeviceId(0x87654321);
    
    mock_driver.register_device(sensor_id);
    mock_driver.register_device(broker_id);
    
    jenlib::ble::BlePayload broadcast_payload;
    broadcast_payload.append_u8(0x04);
    broadcast_payload.append_u8(0x05);
    broadcast_payload.append_u8(0x06);
    
    //! ACT: Test broadcast
    mock_driver.set_local_device_id(broker_id);
    mock_driver.advertise(broker_id, std::move(broadcast_payload));
    
    //! ASSERT: Verify message was broadcast to sensor
    TEST_ASSERT_EQUAL(1, mock_driver.get_message_count(sensor_id));
}

//! @test Validates mock BLE driver connection state simulation
void test_mock_ble_driver_connection_simulation(void) {
    //! ARRANGE: Create and initialize mock BLE driver
    smoke_tests::MockBleDriver mock_driver;
    mock_driver.begin();
    TEST_ASSERT_TRUE(mock_driver.is_connected());
    
    //! ACT: Simulate connection loss
    mock_driver.simulate_connection_loss();
    
    //! ASSERT: Verify connection lost
    TEST_ASSERT_FALSE(mock_driver.is_connected());
    
    //! ACT: Simulate connection restore
    mock_driver.simulate_connection_restore();
    
    //! ASSERT: Verify connection restored
    TEST_ASSERT_TRUE(mock_driver.is_connected());
}

//! @test Validates mock BLE driver cleanup
void test_mock_ble_driver_cleanup(void) {
    //! ARRANGE: Create and initialize mock BLE driver
    smoke_tests::MockBleDriver mock_driver;
    mock_driver.begin();
    TEST_ASSERT_TRUE(mock_driver.is_connected());
    
    //! ACT: End driver
    mock_driver.end();
    
    //! ASSERT: Verify driver is disconnected
    TEST_ASSERT_FALSE(mock_driver.is_connected());
}

//! @section UDP BLE Simulation Tests

//! @test Validates UDP-like point-to-point communication simulation
void test_udp_ble_simulation_point_to_point(void) {
    //! ARRANGE: Initialize mock driver and register devices
    smoke_tests::MockBleDriver mock_driver;
    mock_driver.begin();
    
    const jenlib::ble::DeviceId sensor_id = jenlib::ble::DeviceId(0x12345678);
    const jenlib::ble::DeviceId broker_id = jenlib::ble::DeviceId(0x87654321);
    
    mock_driver.register_device(sensor_id);
    mock_driver.register_device(broker_id);
    mock_driver.set_local_device_id(sensor_id);
    
    jenlib::ble::BlePayload message;
    message.append_u8(0xAA);
    message.append_u8(0xBB);
    message.append_u8(0xCC);
    
    //! ACT: Send message from sensor to broker
    mock_driver.send_to(broker_id, std::move(message));
    
    //! ARRANGE: Switch to broker perspective
    mock_driver.set_local_device_id(broker_id);
    
    //! ACT: Broker receives message
    jenlib::ble::BlePayload received_message;
    bool received = mock_driver.receive(broker_id, received_message);
    
    //! ASSERT: Verify point-to-point message content
    TEST_ASSERT_TRUE(received);
    TEST_ASSERT_EQUAL(3, static_cast<int>(received_message.size));
    TEST_ASSERT_EQUAL(0xAA, received_message.bytes[0]);
    TEST_ASSERT_EQUAL(0xBB, received_message.bytes[1]);
    TEST_ASSERT_EQUAL(0xCC, received_message.bytes[2]);
}

//! @test Validates UDP-like broadcast communication simulation
void test_udp_ble_simulation_broadcast(void) {
    //! ARRANGE: Initialize mock driver and register devices
    smoke_tests::MockBleDriver mock_driver;
    mock_driver.begin();
    
    const jenlib::ble::DeviceId sensor_id = jenlib::ble::DeviceId(0x12345678);
    const jenlib::ble::DeviceId broker_id = jenlib::ble::DeviceId(0x87654321);
    
    mock_driver.register_device(sensor_id);
    mock_driver.register_device(broker_id);
    
    jenlib::ble::BlePayload broadcast_message;
    broadcast_message.append_u8(0xDD);
    broadcast_message.append_u8(0xEE);
    
    //! ACT: Broker broadcasts message
    mock_driver.set_local_device_id(broker_id);
    mock_driver.advertise(broker_id, std::move(broadcast_message));
    
    //! ARRANGE: Switch back to sensor perspective
    mock_driver.set_local_device_id(sensor_id);
    
    //! ACT: Sensor receives broadcast
    jenlib::ble::BlePayload received_broadcast;
    bool received = mock_driver.receive(sensor_id, received_broadcast);
    
    //! ASSERT: Verify broadcast message content
    TEST_ASSERT_TRUE(received);
    TEST_ASSERT_EQUAL(2, static_cast<int>(received_broadcast.size));
    TEST_ASSERT_EQUAL(0xDD, received_broadcast.bytes[0]);
    TEST_ASSERT_EQUAL(0xEE, received_broadcast.bytes[1]);
}

//! @test Validates UDP-like message queuing and processing simulation
void test_udp_ble_simulation_message_queuing(void) {
    //! ARRANGE: Initialize mock driver and register devices
    smoke_tests::MockBleDriver mock_driver;
    mock_driver.begin();
    
    const jenlib::ble::DeviceId sensor_id = jenlib::ble::DeviceId(0x12345678);
    const jenlib::ble::DeviceId broker_id = jenlib::ble::DeviceId(0x87654321);
    
    mock_driver.register_device(sensor_id);
    mock_driver.register_device(broker_id);
    mock_driver.set_local_device_id(broker_id);
    
    //! ACT: Queue multiple messages (simulating network buffering)
    for (int i = 0; i < 5; i++) {
        jenlib::ble::BlePayload queued_message;
        queued_message.append_u8(0x10 + i);
        mock_driver.send_to(sensor_id, std::move(queued_message));
    }
    
    //! ASSERT: Verify messages are queued
    TEST_ASSERT_EQUAL(5, mock_driver.get_message_count(sensor_id));
    
    //! ARRANGE: Switch to sensor perspective
    mock_driver.set_local_device_id(sensor_id);
    
    //! ACT & ASSERT: Process all queued messages
    for (int i = 0; i < 5; i++) {
        jenlib::ble::BlePayload processed_message;
        bool received = mock_driver.receive(sensor_id, processed_message);
        TEST_ASSERT_TRUE(received);
        TEST_ASSERT_EQUAL(1, static_cast<int>(processed_message.size));
        TEST_ASSERT_EQUAL(0x10 + i, processed_message.bytes[0]);
    }
    
    //! ASSERT: Verify queue is empty
    TEST_ASSERT_EQUAL(0, mock_driver.get_message_count(sensor_id));
}

//! @section Mock Sensor Readings Tests

//! @test Validates mock sensor reading functions provide realistic and varied data
void test_mock_sensor_readings(void) {
    //! ARRANGE: Prepare containers for sensor readings
    std::vector<float> temperature_readings;
    std::vector<float> humidity_readings;
    
    //! ACT: Collect multiple readings
    for (int i = 0; i < 10; i++) {
        float temp = smoke_tests::MockSensorReadings::read_temperature_sensor();
        float humidity = smoke_tests::MockSensorReadings::read_humidity_sensor();
        
        temperature_readings.push_back(temp);
        humidity_readings.push_back(humidity);
        
        //! Small delay to allow variation
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    //! ASSERT: Test that readings are within reasonable ranges
    for (float temp : temperature_readings) {
        TEST_ASSERT_GREATER_OR_EQUAL(20.0f, temp);
        TEST_ASSERT_LESS_OR_EQUAL(25.0f, temp);
    }
    
    for (float humidity : humidity_readings) {
        TEST_ASSERT_GREATER_OR_EQUAL(40.0f, humidity);
        TEST_ASSERT_LESS_OR_EQUAL(50.0f, humidity);
    }
    
    //! ACT & ASSERT: Test that readings vary (not all the same)
    bool temp_varies = false;
    bool humidity_varies = false;
    
    for (size_t i = 1; i < temperature_readings.size(); i++) {
        if (temperature_readings[i] != temperature_readings[0]) {
            temp_varies = true;
            break;
        }
    }
    
    for (size_t i = 1; i < humidity_readings.size(); i++) {
        if (humidity_readings[i] != humidity_readings[0]) {
            humidity_varies = true;
            break;
        }
    }
    
    TEST_ASSERT_TRUE(temp_varies);
    TEST_ASSERT_TRUE(humidity_varies);
}

//! @section Mock Broker Behavior Tests

//! @test Validates mock broker initial state
void test_mock_broker_initial_state(void) {
    //! ARRANGE: Initialize mock driver and create broker
    smoke_tests::MockBleDriver mock_driver;
    mock_driver.begin();
    
    const jenlib::ble::DeviceId broker_id = jenlib::ble::DeviceId(0x87654321);
    smoke_tests::MockBroker broker(broker_id, &mock_driver);
    
    //! ACT: No action needed - testing initial state
    
    //! ASSERT: Verify initial state
    TEST_ASSERT_FALSE(broker.is_session_active());
    TEST_ASSERT_EQUAL(0, broker.get_current_session_id().value());
    TEST_ASSERT_EQUAL(0, broker.get_current_sensor_id().value());
}

//! @test Validates mock broker session start functionality
void test_mock_broker_session_start(void) {
    //! ARRANGE: Initialize mock driver and create broker
    smoke_tests::MockBleDriver mock_driver;
    mock_driver.begin();
    
    const jenlib::ble::DeviceId broker_id = jenlib::ble::DeviceId(0x87654321);
    const jenlib::ble::DeviceId sensor_id = jenlib::ble::DeviceId(0x12345678);
    
    mock_driver.register_device(broker_id);
    mock_driver.register_device(sensor_id);
    
    smoke_tests::MockBroker broker(broker_id, &mock_driver);
    
    //! ACT: Start session
    broker.start_session(sensor_id, jenlib::ble::SessionId(0x1234));
    
    //! ASSERT: Verify session started
    TEST_ASSERT_TRUE(broker.is_session_active());
    TEST_ASSERT_EQUAL(0x1234, broker.get_current_session_id().value());
    TEST_ASSERT_EQUAL(sensor_id.value(), broker.get_current_sensor_id().value());
}

//! @test Validates mock broker session start message sending
void test_mock_broker_session_start_message(void) {
    //! ARRANGE: Initialize mock driver and create broker
    smoke_tests::MockBleDriver mock_driver;
    mock_driver.begin();
    
    const jenlib::ble::DeviceId broker_id = jenlib::ble::DeviceId(0x87654321);
    const jenlib::ble::DeviceId sensor_id = jenlib::ble::DeviceId(0x12345678);
    
    mock_driver.register_device(broker_id);
    mock_driver.register_device(sensor_id);
    
    smoke_tests::MockBroker broker(broker_id, &mock_driver);
    
    //! ACT: Start session
    broker.start_session(sensor_id, jenlib::ble::SessionId(0x1234));
    
    //! ARRANGE: Check that start message was sent
    mock_driver.set_local_device_id(sensor_id);
    jenlib::ble::BlePayload received_payload;
    bool received = mock_driver.receive(sensor_id, received_payload);
    
    //! ASSERT: Verify start message received
    TEST_ASSERT_TRUE(received);
    TEST_ASSERT_GREATER_OR_EQUAL(1, static_cast<int>(received_payload.size));
}

//! @test Validates mock broker session stop functionality
void test_mock_broker_session_stop(void) {
    //! ARRANGE: Initialize mock driver and create broker with active session
    smoke_tests::MockBleDriver mock_driver;
    mock_driver.begin();
    
    const jenlib::ble::DeviceId broker_id = jenlib::ble::DeviceId(0x87654321);
    const jenlib::ble::DeviceId sensor_id = jenlib::ble::DeviceId(0x12345678);
    
    mock_driver.register_device(broker_id);
    mock_driver.register_device(sensor_id);
    
    smoke_tests::MockBroker broker(broker_id, &mock_driver);
    broker.start_session(sensor_id, jenlib::ble::SessionId(0x1234));
    TEST_ASSERT_TRUE(broker.is_session_active());
    
    //! ACT: Stop session
    broker.stop_session();
    
    //! ASSERT: Verify session stopped
    TEST_ASSERT_FALSE(broker.is_session_active());
    TEST_ASSERT_EQUAL(0, broker.get_current_session_id().value());
    TEST_ASSERT_EQUAL(0, broker.get_current_sensor_id().value());
}

//! @test Validates mock broker message processing and receipt sending
void test_mock_broker_message_processing(void) {
    //! ARRANGE: Initialize mock driver and create broker with active session
    smoke_tests::MockBleDriver mock_driver;
    mock_driver.begin();
    
    const jenlib::ble::DeviceId broker_id = jenlib::ble::DeviceId(0x87654321);
    const jenlib::ble::DeviceId sensor_id = jenlib::ble::DeviceId(0x12345678);
    
    mock_driver.register_device(broker_id);
    mock_driver.register_device(sensor_id);
    
    smoke_tests::MockBroker broker(broker_id, &mock_driver);
    broker.start_session(sensor_id, jenlib::ble::SessionId(0x5678));
    
    //! ARRANGE: Simulate sensor sending reading message
    mock_driver.set_local_device_id(sensor_id);
    jenlib::ble::BlePayload reading_payload;
    reading_payload.append_u8(0x01); // Mock reading data
    reading_payload.append_u8(0x02);
    reading_payload.append_u8(0x03);
    
    mock_driver.send_to(broker_id, std::move(reading_payload));
    
    //! ACT: Switch to broker perspective and process messages
    mock_driver.set_local_device_id(broker_id);
    broker.process_messages();
    
    //! ARRANGE: Check that receipt was sent back to sensor
    mock_driver.set_local_device_id(sensor_id);
    jenlib::ble::BlePayload receipt_payload;
    bool received = mock_driver.receive(sensor_id, receipt_payload);
    
    //! ASSERT: Verify receipt message received
    TEST_ASSERT_TRUE(received);
    TEST_ASSERT_GREATER_OR_EQUAL(1, static_cast<int>(receipt_payload.size));
}

//! @test Validates mock broker multiple session management
void test_mock_broker_multiple_sessions(void) {
    //! ARRANGE: Initialize mock driver and create broker
    smoke_tests::MockBleDriver mock_driver;
    mock_driver.begin();
    
    const jenlib::ble::DeviceId broker_id = jenlib::ble::DeviceId(0x87654321);
    const jenlib::ble::DeviceId sensor_id = jenlib::ble::DeviceId(0x12345678);
    
    mock_driver.register_device(broker_id);
    mock_driver.register_device(sensor_id);
    
    smoke_tests::MockBroker broker(broker_id, &mock_driver);
    
    //! ACT: Start first session
    broker.start_session(sensor_id, jenlib::ble::SessionId(0x1234));
    TEST_ASSERT_TRUE(broker.is_session_active());
    TEST_ASSERT_EQUAL(0x1234, broker.get_current_session_id().value());
    
    //! ACT: Stop and start new session
    broker.stop_session();
    broker.start_session(sensor_id, jenlib::ble::SessionId(0x9ABC));
    
    //! ASSERT: Verify new session is active
    TEST_ASSERT_TRUE(broker.is_session_active());
    TEST_ASSERT_EQUAL(0x9ABC, broker.get_current_session_id().value());
    TEST_ASSERT_EQUAL(sensor_id.value(), broker.get_current_sensor_id().value());
}

//! @section Test Runner

//! @brief Main function to run all platform mock smoke tests
int main(void) {
    UNITY_BEGIN();
    
    // Mock Time Driver Tests
    RUN_TEST(test_mock_time_driver_initial_state);
    RUN_TEST(test_mock_time_driver_time_advancement);
    RUN_TEST(test_mock_time_driver_time_setting);
    RUN_TEST(test_mock_time_driver_delay);
    RUN_TEST(test_mock_time_driver_reset);
    RUN_TEST(test_mock_time_driver_time_service_integration);
    RUN_TEST(test_mock_time_driver_timer_scheduling);
    
    // Mock BLE Driver Tests
    RUN_TEST(test_mock_ble_driver_initial_state);
    RUN_TEST(test_mock_ble_driver_initialization);
    RUN_TEST(test_mock_ble_driver_device_registration);
    RUN_TEST(test_mock_ble_driver_point_to_point_messaging);
    RUN_TEST(test_mock_ble_driver_broadcast_messaging);
    RUN_TEST(test_mock_ble_driver_connection_simulation);
    RUN_TEST(test_mock_ble_driver_cleanup);
    
    // UDP BLE Simulation Tests
    RUN_TEST(test_udp_ble_simulation_point_to_point);
    RUN_TEST(test_udp_ble_simulation_broadcast);
    RUN_TEST(test_udp_ble_simulation_message_queuing);
    
    // Mock Sensor Readings Tests
    RUN_TEST(test_mock_sensor_readings);
    
    // Mock Broker Behavior Tests
    RUN_TEST(test_mock_broker_initial_state);
    RUN_TEST(test_mock_broker_session_start);
    RUN_TEST(test_mock_broker_session_start_message);
    RUN_TEST(test_mock_broker_session_stop);
    RUN_TEST(test_mock_broker_message_processing);
    RUN_TEST(test_mock_broker_multiple_sessions);
    
    return UNITY_END();
}
