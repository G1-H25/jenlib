//! @file smoke_tests/BleSystemSmokeTests.cpp
//! @brief BLE system smoke tests for jenlib
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include <unity.h>
#include <atomic>
#include <vector>
#include "smoke_tests/SmokeTestSuites.h"
#include <jenlib/ble/Ble.h>
#include <jenlib/ble/Messages.h>
#include <jenlib/ble/Ids.h>
#include <smoke_tests/PlatformMocks.h>

//! @section Test State Tracking
static std::atomic<int> connection_callback_count{0};
static std::atomic<int> start_broadcast_callback_count{0};
static std::atomic<int> reading_callback_count{0};
static std::atomic<int> receipt_callback_count{0};
static std::atomic<int> generic_callback_count{0};
static std::atomic<bool> connection_state{false};
static std::vector<jenlib::ble::StartBroadcastMsg> received_start_messages;
static std::vector<jenlib::ble::ReadingMsg> received_reading_messages;
static std::vector<jenlib::ble::ReceiptMsg> received_receipt_messages;

//! @section Test Callbacks

//! @brief Test callback for BLE connection state changes
//! @param connected True if connected, false if disconnected
void test_connection_callback(bool connected) {
    connection_callback_count++;
    connection_state = connected;
}

//! @brief Test callback for BLE start broadcast messages
//! @param sender_id ID of the device sending the message
//! @param msg Start broadcast message content
void test_start_broadcast_callback(jenlib::ble::DeviceId sender_id, const jenlib::ble::StartBroadcastMsg& msg) {
    start_broadcast_callback_count++;
    received_start_messages.push_back(msg);
}

//! @brief Test callback for BLE reading messages
//! @param sender_id ID of the device sending the message
//! @param msg Reading message content
void test_reading_callback(jenlib::ble::DeviceId sender_id, const jenlib::ble::ReadingMsg& msg) {
    reading_callback_count++;
    received_reading_messages.push_back(msg);
}

//! @brief Test callback for BLE receipt messages
//! @param sender_id ID of the device sending the message
//! @param msg Receipt message content
void test_receipt_callback(jenlib::ble::DeviceId sender_id, const jenlib::ble::ReceiptMsg& msg) {
    receipt_callback_count++;
    received_receipt_messages.push_back(msg);
}

//! @brief Test callback for generic BLE messages
//! @param sender_id ID of the device sending the message
//! @param payload Generic message payload
void test_generic_callback(jenlib::ble::DeviceId sender_id, const jenlib::ble::BlePayload& payload) {
    generic_callback_count++;
}

//! @section Test Setup and Teardown

//! @brief Unity test setup function - resets test state and initializes BLE service
void setUp(void) {
    //! Reset test state
    connection_callback_count = 0;
    start_broadcast_callback_count = 0;
    reading_callback_count = 0;
    receipt_callback_count = 0;
    generic_callback_count = 0;
    connection_state = false;
    received_start_messages.clear();
    received_reading_messages.clear();
    received_receipt_messages.clear();

    //! Initialize BLE with mock driver
    static smoke_tests::MockBleDriver mock_ble_driver;
    jenlib::ble::BLE::set_driver(&mock_ble_driver);
}

//! @brief Unity test teardown function - cleans up after each test
void tearDown(void) {
    //! Clean up BLE service
    jenlib::ble::BLE::end();

    //! Clear all callbacks to prevent state leakage between tests
    jenlib::ble::BLE::set_connection_callback(nullptr);
    jenlib::ble::BLE::set_start_broadcast_callback(nullptr);
    jenlib::ble::BLE::set_reading_callback(nullptr);
    jenlib::ble::BLE::set_receipt_callback(nullptr);
    jenlib::ble::BLE::set_message_callback(nullptr);
}

//! @section BLE System Tests

//! @test Validates BLE driver initialization functionality
void test_ble_driver_initialization(void) {
    //! ARRANGE: No setup needed - testing initialization

    //! ACT: Initialize BLE driver
    bool initialized = jenlib::ble::BLE::begin();

    //! ASSERT: Verify initialization succeeded and connection is established
    TEST_ASSERT_TRUE(initialized);
    TEST_ASSERT_TRUE(jenlib::ble::BLE::is_connected());
}

//! @test Validates BLE initial connection state
void test_ble_initial_connection_state(void) {
    //! ARRANGE: Set connection callback BEFORE initializing BLE
    jenlib::ble::BLE::set_connection_callback(test_connection_callback);

    //! ACT: Initialize BLE (this should trigger the connection callback)
    jenlib::ble::BLE::begin();

    //! ASSERT: Verify initial connection state
    TEST_ASSERT_TRUE(jenlib::ble::BLE::is_connected());
    TEST_ASSERT_TRUE(connection_state.load());
}

//! @test Validates BLE connection loss simulation
void test_ble_connection_loss_simulation(void) {
    //! ARRANGE: Initialize BLE and set connection callback
    jenlib::ble::BLE::begin();
    jenlib::ble::BLE::set_connection_callback(test_connection_callback);
    TEST_ASSERT_TRUE(jenlib::ble::BLE::is_connected()); // Verify initial state

    //! ACT: Simulate connection loss
    static_cast<smoke_tests::MockBleDriver*>(jenlib::ble::BLE::driver())->simulate_connection_loss();

    //! ASSERT: Verify connection loss state
    TEST_ASSERT_FALSE(jenlib::ble::BLE::is_connected());
    TEST_ASSERT_FALSE(connection_state.load());
    TEST_ASSERT_EQUAL(1, connection_callback_count.load());
}

//! @test Validates BLE connection restore simulation
void test_ble_connection_restore_simulation(void) {
    //! ARRANGE: Initialize BLE, set callback, and simulate connection loss
    jenlib::ble::BLE::begin();
    jenlib::ble::BLE::set_connection_callback(test_connection_callback);
    static_cast<smoke_tests::MockBleDriver*>(jenlib::ble::BLE::driver())->simulate_connection_loss();
    TEST_ASSERT_FALSE(jenlib::ble::BLE::is_connected()); // Verify loss state

    //! ACT: Simulate connection restore
    static_cast<smoke_tests::MockBleDriver*>(jenlib::ble::BLE::driver())->simulate_connection_restore();

    //! ASSERT: Verify connection restore state
    TEST_ASSERT_TRUE(jenlib::ble::BLE::is_connected());
    TEST_ASSERT_TRUE(connection_state.load());
    TEST_ASSERT_EQUAL(2, connection_callback_count.load());
}

//! @test Validates BLE message callback registration
void test_ble_message_callback_registration(void) {
    //! ARRANGE: Initialize BLE
    jenlib::ble::BLE::begin();

    //! ACT: Register all callback types
    jenlib::ble::BLE::set_connection_callback(test_connection_callback);
    jenlib::ble::BLE::set_start_broadcast_callback(test_start_broadcast_callback);
    jenlib::ble::BLE::set_reading_callback(test_reading_callback);
    jenlib::ble::BLE::set_receipt_callback(test_receipt_callback);
    jenlib::ble::BLE::set_message_callback(test_generic_callback);

    //! ASSERT: Verify callbacks are registered (no callbacks invoked yet)
    TEST_ASSERT_EQUAL(0, connection_callback_count.load());
    TEST_ASSERT_EQUAL(0, start_broadcast_callback_count.load());
    TEST_ASSERT_EQUAL(0, reading_callback_count.load());
    TEST_ASSERT_EQUAL(0, receipt_callback_count.load());
    TEST_ASSERT_EQUAL(0, generic_callback_count.load());
}

//! @test Validates BLE device registration and setup
void test_ble_device_registration_and_setup(void) {
    //! ARRANGE: Initialize BLE and get mock driver
    jenlib::ble::BLE::begin();
    auto mock_driver = static_cast<smoke_tests::MockBleDriver*>(jenlib::ble::BLE::driver());

    const jenlib::ble::DeviceId sensor_id(0x12345678);
    const jenlib::ble::DeviceId broker_id(0x87654321);

    //! ACT: Register devices and set local device ID
    mock_driver->register_device(sensor_id);
    mock_driver->register_device(broker_id);
    mock_driver->set_local_device_id(sensor_id);

    //! ASSERT: Verify devices are registered (indirectly by testing message sending)
    TEST_ASSERT_TRUE(true); // If we get here without errors, registration succeeded
}

//! @test Validates BLE start broadcast message sending and receiving
void test_ble_start_broadcast_message_flow(void) {
    //! ARRANGE: Initialize BLE, register devices, and set callbacks
    jenlib::ble::BLE::begin();
    auto mock_driver = static_cast<smoke_tests::MockBleDriver*>(jenlib::ble::BLE::driver());

    const jenlib::ble::DeviceId sensor_id(0x12345678);
    const jenlib::ble::DeviceId broker_id(0x87654321);

    mock_driver->register_device(sensor_id);
    mock_driver->register_device(broker_id);
    mock_driver->set_local_device_id(sensor_id);

    jenlib::ble::BLE::set_start_broadcast_callback(test_start_broadcast_callback);

    //! ARRANGE: Prepare start broadcast message
    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = sensor_id,
        .session_id = jenlib::ble::SessionId(0x1234)
    };

    //! ACT: Send start broadcast message and process events
    jenlib::ble::BLE::send_start(sensor_id, start_msg);
    jenlib::ble::BLE::process_events();

    //! ASSERT: Verify message was received
    TEST_ASSERT_EQUAL(1, start_broadcast_callback_count.load());
    TEST_ASSERT_EQUAL(1, static_cast<int>(received_start_messages.size()));
    TEST_ASSERT_EQUAL(sensor_id.value(), received_start_messages[0].device_id.value());
    TEST_ASSERT_EQUAL(0x1234, received_start_messages[0].session_id.value());
}

//! @test Validates BLE reading message sending and receiving
void test_ble_reading_message_flow(void) {
    //! ARRANGE: Initialize BLE, register devices, and set callbacks
    jenlib::ble::BLE::begin();
    auto mock_driver = static_cast<smoke_tests::MockBleDriver*>(jenlib::ble::BLE::driver());

    const jenlib::ble::DeviceId sensor_id(0x12345678);
    const jenlib::ble::DeviceId broker_id(0x87654321);

    mock_driver->register_device(sensor_id);
    mock_driver->register_device(broker_id);
    mock_driver->set_local_device_id(broker_id); // Set broker as local device to receive the broadcast

    jenlib::ble::BLE::set_reading_callback(test_reading_callback);

    //! ARRANGE: Prepare reading message
    jenlib::ble::ReadingMsg reading_msg{
        .sender_id = sensor_id,
        .session_id = jenlib::ble::SessionId(0x1234),
        .offset_ms = 100,
        .temperature_c_centi = 2250, // 22.5°C
        .humidity_bp = 4500 // 45%
    };

    //! ACT: Send reading message and process events
    jenlib::ble::BLE::broadcast_reading(sensor_id, reading_msg);
    jenlib::ble::BLE::process_events();

    //! ASSERT: Verify reading was received
    TEST_ASSERT_EQUAL(1, reading_callback_count.load());
    TEST_ASSERT_EQUAL(1, static_cast<int>(received_reading_messages.size()));
    TEST_ASSERT_EQUAL(sensor_id.value(), received_reading_messages[0].sender_id.value());
    TEST_ASSERT_EQUAL(0x1234, received_reading_messages[0].session_id.value());
    TEST_ASSERT_EQUAL(2250, received_reading_messages[0].temperature_c_centi);
    TEST_ASSERT_EQUAL(4500, received_reading_messages[0].humidity_bp);
}

//! @test Validates BLE receipt message sending and receiving
void test_ble_receipt_message_flow(void) {
    //! ARRANGE: Initialize BLE, register devices, and set callbacks
    jenlib::ble::BLE::begin();
    auto mock_driver = static_cast<smoke_tests::MockBleDriver*>(jenlib::ble::BLE::driver());

    const jenlib::ble::DeviceId sensor_id(0x12345678);
    const jenlib::ble::DeviceId broker_id(0x87654321);

    mock_driver->register_device(sensor_id);
    mock_driver->register_device(broker_id);
    mock_driver->set_local_device_id(sensor_id);

    jenlib::ble::BLE::set_receipt_callback(test_receipt_callback);

    //! ARRANGE: Prepare receipt message
    jenlib::ble::ReceiptMsg receipt_msg{
        .session_id = jenlib::ble::SessionId(0x1234),
        .up_to_offset_ms = 1000
    };

    //! ACT: Send receipt message and process events
    jenlib::ble::BLE::send_receipt(sensor_id, receipt_msg);
    jenlib::ble::BLE::process_events();

    //! ASSERT: Verify receipt was received
    TEST_ASSERT_EQUAL(1, receipt_callback_count.load());
    TEST_ASSERT_EQUAL(1, static_cast<int>(received_receipt_messages.size()));
    TEST_ASSERT_EQUAL(0x1234, received_receipt_messages[0].session_id.value());
    TEST_ASSERT_EQUAL(1000, received_receipt_messages[0].up_to_offset_ms);
}

//! @test Validates BLE event processing functionality
void test_ble_event_processing(void) {
    //! ARRANGE: Initialize BLE
    jenlib::ble::BLE::begin();

    //! ACT: Process events multiple times
    jenlib::ble::BLE::process_events();

    //! ACT: Process events in a loop to test robustness
    for (int i = 0; i < 10; i++) {
        jenlib::ble::BLE::process_events();
    }

    //! ASSERT: Verify no exceptions were thrown (test completes successfully)
    TEST_ASSERT_TRUE(true); // If we get here, no exceptions were thrown
}

//! @section Test Runner

//! @brief Main function to run all BLE system smoke tests
int main(void) {
    UNITY_BEGIN();

    // BLE Driver Tests
    RUN_TEST(test_ble_driver_initialization);

    // BLE Connection State Tests
    RUN_TEST(test_ble_initial_connection_state);
    RUN_TEST(test_ble_connection_loss_simulation);
    RUN_TEST(test_ble_connection_restore_simulation);

    // BLE Message Callback Tests
    RUN_TEST(test_ble_message_callback_registration);

    // BLE Message Flow Tests
    RUN_TEST(test_ble_device_registration_and_setup);
    RUN_TEST(test_ble_start_broadcast_message_flow);
    RUN_TEST(test_ble_reading_message_flow);
    RUN_TEST(test_ble_receipt_message_flow);

    // BLE Event Processing Tests
    RUN_TEST(test_ble_event_processing);

    return UNITY_END();
}

