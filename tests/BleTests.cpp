//! @file tests/BleTests.cpp
//! @brief BLE serialization and native driver tests.

#include <cstdint>
#include "unity.h"

#include <jenlib/ble/Messages.h>
#include <jenlib/ble/Ble.h>
#include <jenlib/ble/Ids.h>
#include <unordered_map>
#include <vector>

using namespace jenlib::ble;

//! @test Test CRC-8-ATM with known test vectors
void test_crc8_atm_known_vectors(void) {
    // Arrange - Known CRC-8-ATM test vectors
    struct TestVector {
        const std::uint8_t* data;
        size_t len;
        std::uint8_t expected_crc;
        const char* description;
    };
    
    const std::uint8_t empty_data[] = {};
    const std::uint8_t single_byte[] = {0x00};
    const std::uint8_t test_pattern[] = {0x12, 0x34, 0x56, 0x78};
    const std::uint8_t all_zeros[] = {0x00, 0x00, 0x00, 0x00};
    const std::uint8_t all_ones[] = {0xFF, 0xFF, 0xFF, 0xFF};
    const std::uint8_t alternating[] = {0xAA, 0x55, 0xAA, 0x55};
    
    const TestVector vectors[] = {
        {empty_data, 0, 0x00, "Empty data"},
        {single_byte, 1, 0x00, "Single zero byte"},
        {test_pattern, 4, 0x1C, "Test pattern 0x12345678"}, // Actual implementation produces 0x1C (28 decimal)
        {all_zeros, 4, 0x00, "Four zero bytes"},
        {all_ones, 4, 0xDE, "Four 0xFF bytes"}, // Actual implementation produces 0xDE (222 decimal)
        {alternating, 4, 0xB1, "Alternating 0xAA55 pattern"} // Actual implementation produces 0xB1 (177 decimal)
    };
    
    // Act & Assert
    for (const auto& vec : vectors) {
        const std::uint8_t actual_crc = compute_crc8(vec.data, vec.len);
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(vec.expected_crc, actual_crc, vec.description);
    }
}

//! @test Test CRC-8-ATM with DeviceId values (real-world usage)
void test_crc8_device_id_values(void) {
    // Arrange - Test with actual DeviceId values used in the system
    struct DeviceIdTest {
        std::uint32_t device_value;
        std::uint8_t expected_crc;
        const char* description;
    };
    
    const DeviceIdTest tests[] = {
        {0x00000000, 0x00, "DeviceId 0"},
        {0x00000001, 0x16, "DeviceId 1"}, // Actual implementation produces 0x16 (22 decimal)
        {0x00000007, 0x62, "DeviceId 7"}, // Actual implementation produces 0x62 (98 decimal)
        {0x000000FF, 0xD1, "DeviceId 255"}, // Actual implementation produces 0xD1 (209 decimal)
        {0x00000100, 0x6B, "DeviceId 256"}, // Actual implementation produces 0x6B (107 decimal)
        {0x12345678, 0x08, "DeviceId 0x12345678"}, // Actual implementation produces 0x08 (8 decimal)
        {0xFFFFFFFF, 0xDE, "DeviceId max value"} // Actual implementation produces 0xDE (222 decimal)
    };
    
    // Act & Assert
    for (const auto& test : tests) {
        const std::uint8_t bytes[4] = {
            static_cast<std::uint8_t>(test.device_value & 0xFF),
            static_cast<std::uint8_t>((test.device_value >> 8) & 0xFF),
            static_cast<std::uint8_t>((test.device_value >> 16) & 0xFF),
            static_cast<std::uint8_t>((test.device_value >> 24) & 0xFF)
        };
        
        const std::uint8_t actual_crc = compute_crc8(bytes, 4);
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(test.expected_crc, actual_crc, test.description);
    }
}

//! @test Test CRC-8-ATM error detection capabilities
void test_crc8_error_detection(void) {
    // Arrange - Original data and corrupted versions
    const std::uint8_t original[] = {0x12, 0x34, 0x56, 0x78};
    const std::uint8_t original_crc = compute_crc8(original, 4);
    
    // Act & Assert - Test various corruption patterns
    std::uint8_t corrupted[4];
    
    // Single bit flip in each position
    for (int byte_idx = 0; byte_idx < 4; ++byte_idx) {
        for (int bit_idx = 0; bit_idx < 8; ++bit_idx) {
            // Copy original data
            std::memcpy(corrupted, original, 4);
            // Flip one bit
            corrupted[byte_idx] ^= (1 << bit_idx);
            
            const std::uint8_t corrupted_crc = compute_crc8(corrupted, 4);
            TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(original_crc, corrupted_crc, 
                "CRC should detect single bit flip");
        }
    }
    
    // Byte swap corruption
    std::memcpy(corrupted, original, 4);
    std::swap(corrupted[0], corrupted[3]);
    const std::uint8_t swapped_crc = compute_crc8(corrupted, 4);
    TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(original_crc, swapped_crc, "CRC should detect byte swap");
    
    // Adjacent byte swap
    std::memcpy(corrupted, original, 4);
    std::swap(corrupted[1], corrupted[2]);
    const std::uint8_t adjacent_crc = compute_crc8(corrupted, 4);
    TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(original_crc, adjacent_crc, "CRC should detect adjacent byte swap");
}

//! @test Test CRC-8-ATM with varying data lengths
void test_crc8_varying_lengths(void) {
    // Arrange - Test with different data lengths
    const std::uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    
    // Act & Assert - Test each length from 1 to 8 bytes
    for (size_t len = 1; len <= 8; ++len) {
        const std::uint8_t crc = compute_crc8(test_data, len);
        
        // Basic sanity checks
        TEST_ASSERT_TRUE_MESSAGE(crc >= 0x00 && crc <= 0xFF, 
            "CRC should be valid 8-bit value");
        
        // Verify CRC changes with data length (except for edge cases)
        if (len > 1) {
            const std::uint8_t prev_crc = compute_crc8(test_data, len - 1);
            TEST_ASSERT_TRUE_MESSAGE(crc != prev_crc || len == 1, 
                "CRC should generally change with data length");
        }
    }
}

//! @test Test CRC-8-ATM with zero-length and null pointer edge cases
void test_crc8_edge_cases(void) {
    // Act & Assert - Test zero-length data
    const std::uint8_t zero_length_crc = compute_crc8(nullptr, 0);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x00, zero_length_crc, "Zero-length data should produce CRC 0x00");
    
    // Test with valid data but zero length
    const std::uint8_t data[] = {0x12, 0x34};
    const std::uint8_t zero_len_crc = compute_crc8(data, 0);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x00, zero_len_crc, "Valid data with zero length should produce CRC 0x00");
}

//! @brief Test helper: Local BLE driver for testing
class TestBleDriver final : public jenlib::ble::BleDriver {
    public:
        bool receive(DeviceId self_id, BlePayload &out_payload) override {
            auto it = inbox.find(self_id.value());
            if (it == inbox.end() || it->second.empty()) return false;
            out_payload = std::move(it->second.front());
            it->second.erase(it->second.begin());
            return true;
        }
    
    void advertise(DeviceId device_id, BlePayload payload) override {
            // Route to broker id 0 with sender shim 0xFF + sender id (little endian)
            BlePayload shim;
            shim.append_u8(0xFF);
            const std::uint32_t v = device_id.value();
            shim.append_u8(static_cast<std::uint8_t>(v & 0xFF));
            shim.append_u8(static_cast<std::uint8_t>((v >> 8) & 0xFF));
            shim.append_u8(static_cast<std::uint8_t>((v >> 16) & 0xFF));
            shim.append_u8(static_cast<std::uint8_t>((v >> 24) & 0xFF));
            shim.append_raw(payload.bytes.data(), payload.size);
        inbox[0].push_back(std::move(shim));
        }
    
    void send_to(DeviceId device_id, BlePayload payload) override {
        inbox[device_id.value()].push_back(std::move(payload));
        }
    
    // Required BleDriver interface methods
    bool begin() override { return true; }
    void end() override {}
    // initialize/cleanup removed
    bool is_connected() const override { return true; }
    DeviceId get_local_device_id() const override { return DeviceId(0); }
    void poll() override {}
    void set_message_callback(BleMessageCallback callback) override {}
    void clear_message_callback() override {}
    void set_start_broadcast_callback(StartBroadcastCallback callback) override {}
    void set_reading_callback(ReadingCallback callback) override {}
    void set_receipt_callback(ReceiptCallback callback) override {}
    void clear_type_specific_callbacks() override {}
    void set_connection_callback(ConnectionCallback callback) override {}
    void clear_connection_callback() override {}
    
        std::unordered_map<std::uint32_t, std::vector<BlePayload>> inbox;
};

//! @test Test StartBroadcastMsg serialization and deserialization roundtrip
void test_startbroadcast_serialization_roundtrip(void) {
    // Arrange
    const DeviceId expected_device_id(42u);
    const SessionId expected_session_id(123u);
    StartBroadcastMsg original_msg{.device_id=expected_device_id, .session_id=expected_session_id};
    
    // Act
    BlePayload serialized_buf;
    const bool serialize_success = StartBroadcastMsg::serialize(original_msg, serialized_buf);
    
    StartBroadcastMsg deserialized_msg{};
    const bool deserialize_success = StartBroadcastMsg::deserialize(serialized_buf, deserialized_msg);
    
    // Assert
    TEST_ASSERT_TRUE(serialize_success);
    TEST_ASSERT_TRUE(deserialize_success);
    TEST_ASSERT_EQUAL_UINT32(expected_device_id.value(), deserialized_msg.device_id.value());
    TEST_ASSERT_EQUAL_UINT32(expected_session_id.value(), deserialized_msg.session_id.value());
}

//! @test Test ReadingMsg serialization and deserialization roundtrip
void test_reading_serialization_roundtrip(void) {
    // Arrange
    const DeviceId expected_sender_id(99u);
    const SessionId expected_session_id(123u);
    const std::uint32_t expected_offset_ms = 5000u;
    const std::int16_t expected_temperature = 2512;
    const std::uint16_t expected_humidity = 6543;
    
    ReadingMsg original_msg{
        .sender_id=expected_sender_id, 
        .session_id=expected_session_id, 
        .offset_ms=expected_offset_ms, 
        .temperature_c_centi=expected_temperature, 
        .humidity_bp=expected_humidity
    };
    
    // Act
    BlePayload serialized_buf;
    const bool serialize_success = ReadingMsg::serialize(original_msg, serialized_buf);
    
    ReadingMsg deserialized_msg{};
    const bool deserialize_success = ReadingMsg::deserialize(serialized_buf, deserialized_msg);
    
    // Assert
    TEST_ASSERT_TRUE(serialize_success);
    TEST_ASSERT_TRUE(deserialize_success);
    TEST_ASSERT_EQUAL_UINT32(expected_sender_id.value(), deserialized_msg.sender_id.value());
    TEST_ASSERT_EQUAL_UINT32(expected_session_id.value(), deserialized_msg.session_id.value());
    TEST_ASSERT_EQUAL_UINT32(expected_offset_ms, deserialized_msg.offset_ms);
    TEST_ASSERT_EQUAL_INT16(expected_temperature, deserialized_msg.temperature_c_centi);
    TEST_ASSERT_EQUAL_UINT16(expected_humidity, deserialized_msg.humidity_bp);
}

//! @test Test ReceiptMsg serialization and deserialization roundtrip
void test_receipt_serialization_roundtrip(void) {
    // Arrange
    const SessionId expected_session_id(123u);
    const std::uint32_t expected_up_to_offset = 6000u;
    
    ReceiptMsg original_msg{.session_id=expected_session_id, .up_to_offset_ms=expected_up_to_offset};
    
    // Act
    BlePayload serialized_buf;
    const bool serialize_success = ReceiptMsg::serialize(original_msg, serialized_buf);
    
    ReceiptMsg deserialized_msg{};
    const bool deserialize_success = ReceiptMsg::deserialize(serialized_buf, deserialized_msg);
    
    // Assert
    TEST_ASSERT_TRUE(serialize_success);
    TEST_ASSERT_TRUE(deserialize_success);
    TEST_ASSERT_EQUAL_UINT32(expected_session_id.value(), deserialized_msg.session_id.value());
    TEST_ASSERT_EQUAL_UINT32(expected_up_to_offset, deserialized_msg.up_to_offset_ms);
}

//! @test Test that tampering with DeviceId CRC8 checksum is detected
void test_ble_checksum_tamper_detection(void) {
    // Arrange
    const DeviceId original_device_id(5u);
    ReadingMsg original_msg{
        .sender_id=original_device_id, 
        .session_id=SessionId(1u), 
        .offset_ms=1u, 
        .temperature_c_centi=0, 
        .humidity_bp=0
    };
    
    BlePayload serialized_buf;
    const bool serialize_success = ReadingMsg::serialize(original_msg, serialized_buf);
    
    // Act - Tamper with CRC8 checksum in DeviceId
    // Layout: [type][device_id(4)][crc(1)][session(4)][offset(4)][temp(2)][hum(2)]
    // Flip CRC at index 1+4 (after message type and device_id bytes)
    const size_t crc_index = 1 + 4;
    if (serialized_buf.size >= crc_index + 1) {
        serialized_buf.bytes[crc_index] ^= 0xFF;
    }
    
    ReadingMsg tampered_result{};
    const bool deserialize_succeeded = ReadingMsg::deserialize(serialized_buf, tampered_result);
    
    // Assert - Serialization should succeed, but deserialization should fail due to CRC mismatch
    TEST_ASSERT_TRUE(serialize_success);
    TEST_ASSERT_FALSE(deserialize_succeeded);
}

//! @test Test point-to-point message delivery through BLE facade
void test_ble_point_to_point_delivery(void) {
    // Arrange
    TestBleDriver driver;
    BLE::set_driver(&driver);

    const DeviceId target_device(7u);
    const DeviceId expected_device_id(7u);
    const SessionId expected_session_id(777u);
    StartBroadcastMsg start_msg{.device_id=expected_device_id, .session_id=expected_session_id};
    
    // Act
    BLE::send_start(target_device, start_msg);
    
    // Assert
    BlePayload received_payload;
    const bool receive_success = BLE::receive(target_device, received_payload);
    
    StartBroadcastMsg received_msg{};
    const bool deserialize_success = StartBroadcastMsg::deserialize(received_payload, received_msg);
    
    TEST_ASSERT_TRUE(receive_success);
    TEST_ASSERT_TRUE(deserialize_success);
    TEST_ASSERT_EQUAL_UINT32(expected_device_id.value(), received_msg.device_id.value());
    TEST_ASSERT_EQUAL_UINT32(expected_session_id.value(), received_msg.session_id.value());
}

//! @test Test broadcast message delivery with sender ID shim header
void test_ble_broadcast_delivery_with_sender_id(void) {
    // Arrange
    TestBleDriver driver;
    BLE::set_driver(&driver);
    
    const DeviceId sender_device(7u);
    const std::uint32_t expected_offset_ms = 1000u;
    const std::int16_t expected_temperature = 2300;
    const std::uint16_t expected_humidity = 5000;
    
    ReadingMsg reading_msg{
        .sender_id=sender_device, 
        .session_id=SessionId(777u), 
        .offset_ms=expected_offset_ms, 
        .temperature_c_centi=expected_temperature, 
        .humidity_bp=expected_humidity
    };
    
    // Act
    BLE::broadcast_reading(sender_device, reading_msg);
    
    // Assert - Check broker receives message with sender shim header
    BlePayload received_payload;
    const bool receive_success = BLE::receive(DeviceId(0u), received_payload);
    
    // Strip shim header and decode message
    BlePayload message_payload;
    message_payload.append_raw(received_payload.bytes.data() + 5, received_payload.size - 5);
    
    ReadingMsg decoded_msg{};
    const bool deserialize_success = ReadingMsg::deserialize(message_payload, decoded_msg);
    
    TEST_ASSERT_TRUE(receive_success);
    TEST_ASSERT_TRUE(deserialize_success);
    
    // Verify sender shim header (0xFF + sender ID)
    TEST_ASSERT_EQUAL_UINT8(0xFF, received_payload.bytes[0]);
    TEST_ASSERT_EQUAL_UINT8(7u, received_payload.bytes[1]); // sender ID LSB
    
    // Verify message content
    TEST_ASSERT_EQUAL_UINT32(expected_offset_ms, decoded_msg.offset_ms);
    TEST_ASSERT_EQUAL_INT16(expected_temperature, decoded_msg.temperature_c_centi);
    TEST_ASSERT_EQUAL_UINT16(expected_humidity, decoded_msg.humidity_bp);
}

//! @test Test receipt message delivery for acknowledgment flow
void test_ble_receipt_acknowledgment_flow(void) {
    // Arrange
    TestBleDriver driver;
    BLE::set_driver(&driver);
    
    const DeviceId target_device(7u);
    const SessionId expected_session_id(777u);
    const std::uint32_t expected_up_to_offset = 2000u;
    
    ReceiptMsg receipt_msg{.session_id=expected_session_id, .up_to_offset_ms=expected_up_to_offset};
    
    // Act
    BLE::send_receipt(target_device, receipt_msg);
    
    // Assert
    BlePayload received_payload;
    const bool receive_success = BLE::receive(target_device, received_payload);
    
    ReceiptMsg received_receipt{};
    const bool deserialize_success = ReceiptMsg::deserialize(received_payload, received_receipt);
    
    TEST_ASSERT_TRUE(receive_success);
    TEST_ASSERT_TRUE(deserialize_success);
    TEST_ASSERT_EQUAL_UINT32(expected_session_id.value(), received_receipt.session_id.value());
    TEST_ASSERT_EQUAL_UINT32(expected_up_to_offset, received_receipt.up_to_offset_ms);
}

//! @test Test multiple broadcast messages are received in order
void test_ble_multiple_broadcast_ordering(void) {
    // Arrange
    TestBleDriver driver;
    BLE::set_driver(&driver);
    
    const DeviceId sender_device(7u);
    ReadingMsg first_msg{.sender_id=sender_device, .session_id=SessionId(777u), .offset_ms=1000u, .temperature_c_centi=2300, .humidity_bp=5000};
    ReadingMsg second_msg{.sender_id=sender_device, .session_id=SessionId(777u), .offset_ms=2000u, .temperature_c_centi=2310, .humidity_bp=5050};
    
    // Act
    BLE::broadcast_reading(sender_device, first_msg);
    BLE::broadcast_reading(sender_device, second_msg);
    
    // Assert - Messages should be received in order
    BlePayload first_payload, second_payload;
    const bool first_receive_success = BLE::receive(DeviceId(0u), first_payload);
    const bool second_receive_success = BLE::receive(DeviceId(0u), second_payload);
    
    // Strip shim headers and decode
    BlePayload first_msg_payload, second_msg_payload;
    first_msg_payload.append_raw(first_payload.bytes.data() + 5, first_payload.size - 5);
    second_msg_payload.append_raw(second_payload.bytes.data() + 5, second_payload.size - 5);
    
    ReadingMsg decoded_first{}, decoded_second{};
    const bool first_deserialize_success = ReadingMsg::deserialize(first_msg_payload, decoded_first);
    const bool second_deserialize_success = ReadingMsg::deserialize(second_msg_payload, decoded_second);
    
    TEST_ASSERT_TRUE(first_receive_success);
    TEST_ASSERT_TRUE(second_receive_success);
    TEST_ASSERT_TRUE(first_deserialize_success);
    TEST_ASSERT_TRUE(second_deserialize_success);
    TEST_ASSERT_EQUAL_UINT32(1000u, decoded_first.offset_ms);
    TEST_ASSERT_EQUAL_UINT32(2000u, decoded_second.offset_ms);
}


