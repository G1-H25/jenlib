//! @file tests/BleInterfaceContractTests.cpp
//! @brief Interface contract compliance tests for BleDriver implementations.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include <unity.h>
#include <cstdint>
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <utility>
#include <jenlib/ble/BleDriver.h>
#include <jenlib/ble/drivers/NativeBleDriver.h>

using namespace jenlib::ble;

//! @brief Test helper: Focused unit test utilities
class BleDriverTestUtils {
public:
    //! @brief Create a test payload with known content
    static BlePayload create_test_payload() {
        BlePayload payload;
        payload.append_u8(0x01); // Message type
        payload.append_u32le(0x12345678); // Test data
        return payload;
    }

    //! @brief Verify payload content matches expected values
    static void verify_test_payload(const BlePayload& payload) {
        TEST_ASSERT_EQUAL_UINT8(0x01, payload.bytes[0]);
        std::size_t i = 1; std::uint32_t v = 0; TEST_ASSERT_TRUE(jenlib::ble::read_u32le(payload, i, v));
        TEST_ASSERT_EQUAL_UINT32(0x12345678, v);
    }
};

//! @test Test driver initialization returns expected state
void test_driver_initialization_state(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));

    //! Act
    bool init_result = driver.begin();

    //! Assert
    TEST_ASSERT_TRUE(init_result);
    TEST_ASSERT_TRUE(driver.is_connected());
    TEST_ASSERT_EQUAL_UINT32(0x12345678, driver.get_local_device_id().value());
}

//! @test Test driver can be initialized and cleaned up multiple times
void test_driver_lifecycle_reinitialization(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));

    //! Act - First initialization
    bool init1 = driver.begin();
    bool connected1 = driver.is_connected();
    driver.end();
    bool connected_after_cleanup = driver.is_connected();

    //! Act - Second initialization
    bool init2 = driver.begin();
    bool connected2 = driver.is_connected();

    //! Assert
    TEST_ASSERT_TRUE(init1);
    TEST_ASSERT_TRUE(init2);
    TEST_ASSERT_TRUE(connected1);
    TEST_ASSERT_FALSE(connected_after_cleanup);
    TEST_ASSERT_TRUE(connected2);
}

//! @test Test messaging round-trip with payload verification
void test_messaging_round_trip_with_payload(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();

    BlePayload test_payload = BleDriverTestUtils::create_test_payload();
    DeviceId target_device(0x11111111);

    //! Act
    driver.send_to(target_device, test_payload);
    BlePayload received;
    bool receive_result = driver.receive(target_device, received);

    //! Assert
    TEST_ASSERT_TRUE(receive_result);
    BleDriverTestUtils::verify_test_payload(received);
}

//! @test Test callback is invoked when message is sent
void test_callback_invocation_on_message(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();
    std::atomic<int> callback_count{0};
    driver.set_message_callback([&callback_count](DeviceId, const BlePayload&) {
        callback_count++;
    });

    //! Act
    BlePayload test_payload = BleDriverTestUtils::create_test_payload();
    driver.send_to(DeviceId(0x11111111), std::move(test_payload));

    //! Assert
    TEST_ASSERT_EQUAL_INT(1, callback_count.load());
}

//! @test Test callback is not invoked after being cleared
void test_callback_not_invoked_after_clearing(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();
    std::atomic<int> callback_count{0};
    driver.set_message_callback([&callback_count](DeviceId, const BlePayload&) {
        callback_count++;
    });
    driver.clear_message_callback();

    //! Act
    BlePayload test_payload = BleDriverTestUtils::create_test_payload();
    driver.send_to(DeviceId(0x11111111), std::move(test_payload));

    //! Assert
    TEST_ASSERT_EQUAL_INT(0, callback_count.load());
}

//! @test Test initial driver state
void test_initial_driver_state(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));

    //! Act & Assert
    TEST_ASSERT_FALSE(driver.is_connected());
    TEST_ASSERT_EQUAL_UINT32(0x12345678, driver.get_local_device_id().value());
}

//! @test Test driver state after initialization
void test_driver_state_after_initialization(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));

    //! Act
    driver.begin();

    //! Assert
    TEST_ASSERT_TRUE(driver.is_connected());
}

//! @test Test driver state after cleanup
void test_driver_state_after_cleanup(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();

    //! Act
    driver.end();

    //! Assert
    TEST_ASSERT_FALSE(driver.is_connected());
}

//! @test Test send fails when not initialized
void test_send_fails_when_not_initialized(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));
    BlePayload test_payload = BleDriverTestUtils::create_test_payload();

    //! Act
    driver.send_to(DeviceId(0x11111111), std::move(test_payload));

    //! Assert
    TEST_ASSERT_FALSE(driver.is_connected());
}

//! @test Test receive fails when not initialized
void test_receive_fails_when_not_initialized(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));

    //! Act
    BlePayload received;
    bool receive_result = driver.receive(DeviceId(0x12345678), received);

    //! Assert
    TEST_ASSERT_FALSE(receive_result);
    TEST_ASSERT_FALSE(driver.is_connected());
}

//! @test Test concurrent message sending and receiving
void test_concurrent_messaging(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();

    std::atomic<int> messages_sent{0};
    std::atomic<int> messages_received{0};
    std::atomic<bool> test_complete{false};

    //! Act - Create sender thread
    std::thread sender([&driver, &messages_sent, &test_complete]() {
        while (!test_complete.load()) {
            BlePayload payload = BleDriverTestUtils::create_test_payload();
            driver.send_to(DeviceId(0x11111111), std::move(payload));
            messages_sent++;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    //! Act - Create receiver thread
    std::thread receiver([&driver, &messages_received, &test_complete]() {
        while (!test_complete.load()) {
            BlePayload received;
            if (driver.receive(DeviceId(0x11111111), received)) {
                messages_received++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    //! Let threads run for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    test_complete.store(true);

    sender.join();
    receiver.join();

    //! Assert
    TEST_ASSERT_TRUE(messages_sent.load() > 0);
    TEST_ASSERT_TRUE(messages_received.load() > 0);
    TEST_ASSERT_TRUE(messages_received.load() <= messages_sent.load());
}

//! @test Test messaging with zero device ID
void test_messaging_with_zero_device_id(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();
    BlePayload test_payload = BleDriverTestUtils::create_test_payload();
    DeviceId device(0x00000000);

    //! Act
    driver.send_to(device, test_payload);
    BlePayload received;
    bool result = driver.receive(device, received);

    //! Assert
    TEST_ASSERT_TRUE(result);
    BleDriverTestUtils::verify_test_payload(received);
}

//! @test Test messaging with max device ID
void test_messaging_with_max_device_id(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();
    BlePayload test_payload = BleDriverTestUtils::create_test_payload();
    DeviceId device(0xFFFFFFFF);

    //! Act
    driver.send_to(device, test_payload);
    BlePayload received;
    bool result = driver.receive(device, received);

    //! Assert
    TEST_ASSERT_TRUE(result);
    BleDriverTestUtils::verify_test_payload(received);
}

//! @test Test messaging with normal device ID
void test_messaging_with_normal_device_id(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();
    BlePayload test_payload = BleDriverTestUtils::create_test_payload();
    DeviceId device(0x12345678);

    //! Act
    driver.send_to(device, test_payload);
    BlePayload received;
    bool result = driver.receive(device, received);

    //! Assert
    TEST_ASSERT_TRUE(result);
    BleDriverTestUtils::verify_test_payload(received);
}

//! @test Test callback parameters are passed correctly
void test_callback_parameters_passed_correctly(void) {
    // Arrange
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();

    DeviceId expected_sender(0x11111111);
    BlePayload expected_payload = BleDriverTestUtils::create_test_payload();

    DeviceId actual_sender(0x00000000);
    BlePayload actual_payload;
    bool callback_called = false;

    // Act - Set callback and send message
    driver.set_message_callback([&](DeviceId sender_id, const BlePayload& payload) {
        actual_sender = sender_id;
        actual_payload = payload;
        callback_called = true;
    });

    driver.send_to(expected_sender, expected_payload);

    // Assert
    TEST_ASSERT_TRUE(callback_called);
    TEST_ASSERT_EQUAL_UINT32(expected_sender.value(), actual_sender.value());
    BleDriverTestUtils::verify_test_payload(actual_payload);
}

//! @test Test initialization return value consistency
void test_initialization_return_value_consistency(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));

    //! Act
    bool init1 = driver.begin();
    driver.end();
    bool init2 = driver.begin();

    //! Assert
    TEST_ASSERT_EQUAL_UINT8(init1, init2);
}

//! @test Test connection state return value consistency
void test_connection_state_return_value_consistency(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));

    //! Act
    bool connected1 = driver.is_connected();
    driver.begin();
    bool connected2 = driver.is_connected();

    //! Assert
    TEST_ASSERT_EQUAL_UINT8(connected1, connected2);
}

//! @test Test device ID return value consistency
void test_device_id_return_value_consistency(void) {
    //! Arrange
    NativeBleDriver driver(DeviceId(0x12345678));

    //! Act
    DeviceId id1 = driver.get_local_device_id();
    DeviceId id2 = driver.get_local_device_id();

    //! Assert
    TEST_ASSERT_EQUAL_UINT32(id1.value(), id2.value());
}

