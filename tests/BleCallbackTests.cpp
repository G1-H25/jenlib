//! @file tests/BleCallbackTests.cpp
//! @brief Comprehensive tests for BLE callback system and interface contract.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)
//!
//! Tests cover:
//! - Type-specific callback functionality
//! - Message routing and priority
//! - Interface contract compliance
//! - Error handling and edge cases
//! - Sender ID extraction

#include <unity.h>
#include <cstdint>
#include <string>
#include <utility>
#include <memory>
#include <vector>
#include <atomic>
#include "jenlib/ble/BleDriver.h"
#include "jenlib/ble/Messages.h"
#include "jenlib/ble/drivers/NativeBleDriver.h"

using jenlib::ble::BlePayload;
using jenlib::ble::DeviceId;
using jenlib::ble::NativeBleDriver;
using jenlib::ble::ReadingMsg;
using jenlib::ble::ReceiptMsg;
using jenlib::ble::SessionId;
using jenlib::ble::StartBroadcastMsg;

//! @brief Test helper: Mock callback tracker
class CallbackTracker {
 public:
    struct CallbackCall {
        DeviceId sender_id;
        std::string message_type;
        std::uint32_t session_id;
        std::uint32_t offset_ms;
        std::int16_t temperature;
        std::uint16_t humidity;
    };

    std::vector<CallbackCall> start_broadcast_calls;
    std::vector<CallbackCall> reading_calls;
    std::vector<CallbackCall> receipt_calls;
    std::vector<CallbackCall> generic_calls;

    void clear() {
        start_broadcast_calls.clear();
        reading_calls.clear();
        receipt_calls.clear();
        generic_calls.clear();
    }

    // Type-specific callbacks
    void on_start_broadcast(DeviceId sender_id, const StartBroadcastMsg& msg) {
        start_broadcast_calls.push_back({
            sender_id, "StartBroadcast", msg.session_id.value(), 0, 0, 0
        });
    }

    void on_reading(DeviceId sender_id, const ReadingMsg& msg) {
        reading_calls.push_back({
            sender_id, "Reading", msg.session_id.value(), msg.offset_ms,
            msg.temperature_c_centi, msg.humidity_bp
        });
    }

    void on_receipt(DeviceId sender_id, const ReceiptMsg& msg) {
        receipt_calls.push_back({
            sender_id, "Receipt", msg.session_id.value(), msg.up_to_offset_ms, 0, 0
        });
    }

    // Generic callback
    void on_generic_message(DeviceId sender_id, const BlePayload& payload) {
        generic_calls.push_back({
            sender_id, "Generic", 0, 0, 0, 0
        });
    }
};

//! @test Test type-specific callback registration and invocation
void test_type_specific_callback_registration(void) {
    //! @section Arrange
    CallbackTracker tracker;
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();

    //! @section Act - Register callbacks and send one message of each type
    driver.set_start_broadcast_callback([&tracker](DeviceId sender_id, const StartBroadcastMsg& msg) {
        tracker.on_start_broadcast(sender_id, msg);
    });
    driver.set_reading_callback([&tracker](DeviceId sender_id, const ReadingMsg& msg) {
        tracker.on_reading(sender_id, msg);
    });
    driver.set_receipt_callback([&tracker](DeviceId sender_id, const ReceiptMsg& msg) {
        tracker.on_receipt(sender_id, msg);
    });

    // Create and send StartBroadcast
    StartBroadcastMsg start_msg;
    start_msg.device_id = DeviceId(0x87654321);
    start_msg.session_id = SessionId(0xAAAAAAAA);
    BlePayload start_payload;
    StartBroadcastMsg::serialize(start_msg, start_payload);
    driver.send_to(DeviceId(0x12345678), std::move(start_payload));

    // Create and send Reading
    ReadingMsg reading_msg;
    reading_msg.sender_id = DeviceId(0x87654321);
    reading_msg.session_id = SessionId(0xBBBBBBBB);
    reading_msg.offset_ms = 1;
    reading_msg.temperature_c_centi = 1;
    reading_msg.humidity_bp = 1;
    BlePayload reading_payload;
    ReadingMsg::serialize(reading_msg, reading_payload);
    driver.send_to(DeviceId(0x12345678), std::move(reading_payload));

    // Create and send Receipt
    ReceiptMsg receipt_msg;
    receipt_msg.session_id = SessionId(0xCCCCCCCC);
    receipt_msg.up_to_offset_ms = 0;
    BlePayload receipt_payload;
    ReceiptMsg::serialize(receipt_msg, receipt_payload);
    driver.send_to(DeviceId(0x12345678), std::move(receipt_payload));

    //! @section Assert - Each registered callback should have been invoked exactly once
    TEST_ASSERT_EQUAL_UINT32(1, tracker.start_broadcast_calls.size());
    TEST_ASSERT_EQUAL_UINT32(1, tracker.reading_calls.size());
    TEST_ASSERT_EQUAL_UINT32(1, tracker.receipt_calls.size());
}

//! @test Test StartBroadcast message routing to type-specific callback
void test_start_broadcast_callback_routing(void) {
    //! @section Arrange
    CallbackTracker tracker;
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();

    driver.set_start_broadcast_callback([&tracker](DeviceId sender_id, const StartBroadcastMsg& msg) {
        tracker.on_start_broadcast(sender_id, msg);
    });

    // Create StartBroadcast message
    StartBroadcastMsg start_msg;
    start_msg.device_id = DeviceId(0x87654321);
    start_msg.session_id = SessionId(0x11111111);

    BlePayload payload;
    StartBroadcastMsg::serialize(start_msg, payload);

    //! @section Act - Send message to driver
    driver.send_to(DeviceId(0x12345678), std::move(payload));

    //! @section Assert - Callback should be invoked
    TEST_ASSERT_EQUAL_UINT32(1, tracker.start_broadcast_calls.size());
    TEST_ASSERT_EQUAL_UINT32(0x87654321, tracker.start_broadcast_calls[0].sender_id.value());
    TEST_ASSERT_EQUAL_UINT32(0x11111111, tracker.start_broadcast_calls[0].session_id);
    TEST_ASSERT_EQUAL_STRING("StartBroadcast", tracker.start_broadcast_calls[0].message_type.c_str());
}

//! @test Test Reading message routing to type-specific callback
void test_reading_callback_routing(void) {
    //! @section Arrange
    CallbackTracker tracker;
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();

    driver.set_reading_callback([&tracker](DeviceId sender_id, const ReadingMsg& msg) {
        tracker.on_reading(sender_id, msg);
    });

    // Create Reading message
    ReadingMsg reading_msg;
    reading_msg.sender_id = DeviceId(0x87654321);
    reading_msg.session_id = SessionId(0x22222222);
    reading_msg.offset_ms = 5000;
    reading_msg.temperature_c_centi = 2312;  // 23.12°C
    reading_msg.humidity_bp = 4567;          // 45.67%

    BlePayload payload;
    ReadingMsg::serialize(reading_msg, payload);

    //! @section Act - Send message to driver
    driver.send_to(DeviceId(0x12345678), std::move(payload));

    //! @section Assert - Callback should be invoked
    TEST_ASSERT_EQUAL_UINT32(1, tracker.reading_calls.size());
    TEST_ASSERT_EQUAL_UINT32(0x87654321, tracker.reading_calls[0].sender_id.value());
    TEST_ASSERT_EQUAL_UINT32(0x22222222, tracker.reading_calls[0].session_id);
    TEST_ASSERT_EQUAL_UINT32(5000, tracker.reading_calls[0].offset_ms);
    TEST_ASSERT_EQUAL_INT16(2312, tracker.reading_calls[0].temperature);
    TEST_ASSERT_EQUAL_UINT16(4567, tracker.reading_calls[0].humidity);
    TEST_ASSERT_EQUAL_STRING("Reading", tracker.reading_calls[0].message_type.c_str());
}

//! @test Test Receipt message routing to type-specific callback
void test_receipt_callback_routing(void) {
    //! @section Arrange
    CallbackTracker tracker;
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();

    driver.set_receipt_callback([&tracker](DeviceId sender_id, const ReceiptMsg& msg) {
        tracker.on_receipt(sender_id, msg);
    });

    // Create Receipt message
    ReceiptMsg receipt_msg;
    receipt_msg.session_id = SessionId(0x33333333);
    receipt_msg.up_to_offset_ms = 10000;

    BlePayload payload;
    ReceiptMsg::serialize(receipt_msg, payload);

    //! @section Act - Send message to driver
    driver.send_to(DeviceId(0x12345678), std::move(payload));

    //! @section Assert - Callback should be invoked
    TEST_ASSERT_EQUAL_UINT32(1, tracker.receipt_calls.size());
    TEST_ASSERT_EQUAL_UINT32(0x33333333, tracker.receipt_calls[0].session_id);
    TEST_ASSERT_EQUAL_UINT32(10000, tracker.receipt_calls[0].offset_ms);
    TEST_ASSERT_EQUAL_STRING("Receipt", tracker.receipt_calls[0].message_type.c_str());
}

//! @test Test callback priority: type-specific over generic
void test_callback_priority_type_specific_over_generic(void) {
    //! @section Arrange
    CallbackTracker tracker;
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();

    // Register both type-specific and generic callbacks
    driver.set_start_broadcast_callback([&tracker](DeviceId sender_id, const StartBroadcastMsg& msg) {
        tracker.on_start_broadcast(sender_id, msg);
    });
    driver.set_message_callback([&tracker](DeviceId sender_id, const BlePayload& payload) {
        tracker.on_generic_message(sender_id, payload);
    });

    // Create StartBroadcast message
    StartBroadcastMsg start_msg;
    start_msg.device_id = DeviceId(0x87654321);
    start_msg.session_id = SessionId(0x44444444);

    BlePayload payload;
    StartBroadcastMsg::serialize(start_msg, payload);

    //! @section Act - Send message to driver
    driver.send_to(DeviceId(0x12345678), std::move(payload));

    //! @section Assert - Only type-specific callback should be invoked
    TEST_ASSERT_EQUAL_UINT32(1, tracker.start_broadcast_calls.size());
    TEST_ASSERT_EQUAL_UINT32(0, tracker.generic_calls.size());
}

//! @test Test fallback to generic callback when no type-specific callback
void test_fallback_to_generic_callback(void) {
    //! @section Arrange
    CallbackTracker tracker;
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();

    // Register only generic callback
    driver.set_message_callback([&tracker](DeviceId sender_id, const BlePayload& payload) {
        tracker.on_generic_message(sender_id, payload);
    });

    // Create StartBroadcast message
    StartBroadcastMsg start_msg;
    start_msg.device_id = DeviceId(0x87654321);
    start_msg.session_id = SessionId(0x55555555);

    BlePayload payload;
    StartBroadcastMsg::serialize(start_msg, payload);

    //! @section Act - Send message to driver
    driver.send_to(DeviceId(0x12345678), std::move(payload));

    //! @section Assert - Generic callback should be invoked
    TEST_ASSERT_EQUAL_UINT32(0, tracker.start_broadcast_calls.size());
    TEST_ASSERT_EQUAL_UINT32(1, tracker.generic_calls.size());
}

//! @test Test callback clearing functionality
void test_callback_clearing(void) {
    //! @section Arrange
    CallbackTracker tracker;
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();

    // Register callbacks
    driver.set_start_broadcast_callback([&tracker](DeviceId sender_id, const StartBroadcastMsg& msg) {
        tracker.on_start_broadcast(sender_id, msg);
    });
    driver.set_reading_callback([&tracker](DeviceId sender_id, const ReadingMsg& msg) {
        tracker.on_reading(sender_id, msg);
    });
    driver.set_receipt_callback([&tracker](DeviceId sender_id, const ReceiptMsg& msg) {
        tracker.on_receipt(sender_id, msg);
    });

    //! @section Act - Clear type-specific callbacks
    driver.clear_type_specific_callbacks();

    // Create and send messages
    StartBroadcastMsg start_msg;
    start_msg.device_id = DeviceId(0x87654321);
    start_msg.session_id = SessionId(0x66666666);

    BlePayload payload;
    StartBroadcastMsg::serialize(start_msg, payload);
    driver.send_to(DeviceId(0x12345678), std::move(payload));

    //! @section Assert - No callbacks should be invoked
    TEST_ASSERT_EQUAL_UINT32(0, tracker.start_broadcast_calls.size());
    TEST_ASSERT_EQUAL_UINT32(0, tracker.reading_calls.size());
    TEST_ASSERT_EQUAL_UINT32(0, tracker.receipt_calls.size());
}

//! @test Test multiple message types with different callbacks
void test_multiple_message_types_different_callbacks(void) {
    //! @section Arrange
    CallbackTracker tracker;
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();

    // Register different callbacks for different message types
    driver.set_start_broadcast_callback([&tracker](DeviceId sender_id, const StartBroadcastMsg& msg) {
        tracker.on_start_broadcast(sender_id, msg);
    });
    driver.set_reading_callback([&tracker](DeviceId sender_id, const ReadingMsg& msg) {
        tracker.on_reading(sender_id, msg);
    });
    // No receipt callback registered

    // Create messages
    StartBroadcastMsg start_msg;
    start_msg.device_id = DeviceId(0x87654321);
    start_msg.session_id = SessionId(0x77777777);

    ReadingMsg reading_msg;
    reading_msg.sender_id = DeviceId(0x87654321);
    reading_msg.session_id = SessionId(0x88888888);
    reading_msg.offset_ms = 1000;
    reading_msg.temperature_c_centi = 2500;
    reading_msg.humidity_bp = 5000;

    ReceiptMsg receipt_msg;
    receipt_msg.session_id = SessionId(0x99999999);
    receipt_msg.up_to_offset_ms = 2000;

    //! @section Act - Send all messages
    BlePayload start_payload, reading_payload, receipt_payload;
    StartBroadcastMsg::serialize(start_msg, start_payload);
    ReadingMsg::serialize(reading_msg, reading_payload);
    ReceiptMsg::serialize(receipt_msg, receipt_payload);

    driver.send_to(DeviceId(0x12345678), std::move(start_payload));
    driver.send_to(DeviceId(0x12345678), std::move(reading_payload));
    driver.send_to(DeviceId(0x12345678), std::move(receipt_payload));

    //! @section Assert - Only registered callbacks should be invoked
    TEST_ASSERT_EQUAL_UINT32(1, tracker.start_broadcast_calls.size());
    TEST_ASSERT_EQUAL_UINT32(1, tracker.reading_calls.size());
    TEST_ASSERT_EQUAL_UINT32(0, tracker.receipt_calls.size());
}

//! @test Test callback with invalid message data
void test_callback_with_invalid_message_data(void) {
    //! @section Arrange
    CallbackTracker tracker;
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();

    driver.set_start_broadcast_callback([&tracker](DeviceId sender_id, const StartBroadcastMsg& msg) {
        tracker.on_start_broadcast(sender_id, msg);
    });

    // Create invalid payload (too small for StartBroadcast)
    BlePayload invalid_payload;
    invalid_payload.append_u8(0x01);  // Message type
    invalid_payload.append_u8(0x12);  // Only partial device ID

    //! @section Act - Send invalid message
    driver.send_to(DeviceId(0x12345678), std::move(invalid_payload));

    //! @section Assert - Callback should not be invoked due to deserialization failure
    TEST_ASSERT_EQUAL_UINT32(0, tracker.start_broadcast_calls.size());
}

//! @test Test interface contract compliance: all virtual methods implemented
void test_interface_contract_compliance(void) {
    //! @section Arrange
    NativeBleDriver driver(DeviceId(0x12345678));

    //! @section Act - Call all interface methods
    bool begin_result = driver.begin();
    bool init_result = false;  // initialize removed
    bool connected = driver.is_connected();
    DeviceId local_id = driver.get_local_device_id();

    // Test callback methods
    driver.set_message_callback([](DeviceId, const BlePayload&) {});
    driver.set_start_broadcast_callback([](DeviceId, const StartBroadcastMsg&) {});
    driver.set_reading_callback([](DeviceId, const ReadingMsg&) {});
    driver.set_receipt_callback([](DeviceId, const ReceiptMsg&) {});
    driver.clear_message_callback();
    driver.clear_type_specific_callbacks();

    // Test lifecycle methods
    driver.end();
    driver.end();
    driver.poll();

    // Test messaging methods
    BlePayload empty_payload;
    driver.advertise(DeviceId(0x11111111), BlePayload{});
    driver.send_to(DeviceId(0x22222222), BlePayload{});

    BlePayload received;
    bool receive_result = driver.receive(DeviceId(0x12345678), received);

    //! @section Assert - All methods should return expected values
    TEST_ASSERT_FALSE(begin_result);
    TEST_ASSERT_FALSE(init_result);
    TEST_ASSERT_FALSE(connected);
    TEST_ASSERT_EQUAL_UINT32(0x12345678, local_id.value());
    TEST_ASSERT_FALSE(receive_result);
}

//! @test Test sender ID extraction from payload
void test_sender_id_extraction(void) {
    //! @section Arrange
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();

    ReadingMsg reading_msg;
    reading_msg.sender_id = DeviceId(0x87654321);
    reading_msg.session_id = SessionId(0x11111111);
    reading_msg.offset_ms = 1000;
    reading_msg.temperature_c_centi = 2500;
    reading_msg.humidity_bp = 5000;

    BlePayload payload;
    ReadingMsg::serialize(reading_msg, payload);

    //! @section Act - Send message (this will add sender ID marker)
    driver.advertise(DeviceId(0x87654321), std::move(payload));

    BlePayload received;
    bool receive_success = driver.receive(DeviceId(0), received);  // Broker receives broadcasts

    //! @section Assert - Check that sender ID is extracted correctly
    TEST_ASSERT_TRUE(receive_success);
    TEST_ASSERT_EQUAL_UINT8(0xFF, received.bytes[0]);  // Sender ID marker
    TEST_ASSERT_EQUAL_UINT8(0x21, received.bytes[1]);  // Sender ID LSB
    TEST_ASSERT_EQUAL_UINT8(0x43, received.bytes[2]);
    TEST_ASSERT_EQUAL_UINT8(0x65, received.bytes[3]);
    TEST_ASSERT_EQUAL_UINT8(0x87, received.bytes[4]);  // Sender ID MSB
}

//! @test Test callback error handling and recovery
void test_callback_error_handling(void) {
    //! @section Arrange
    std::atomic<int> callback_count{0};
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();

    driver.set_start_broadcast_callback([&callback_count](DeviceId, const StartBroadcastMsg&) {
        callback_count++;
        throw std::runtime_error("Callback error");
    });

    StartBroadcastMsg start_msg;
    start_msg.device_id = DeviceId(0x87654321);
    start_msg.session_id = SessionId(0x11111111);

    BlePayload payload;
    StartBroadcastMsg::serialize(start_msg, payload);

    //! @section Act - Send message (callback should throw)
    try {
        driver.send_to(DeviceId(0x12345678), std::move(payload));
    } catch (...) {
        //  Driver should handle exceptions gracefully
    }

    //! @section Assert - Callback should have been called
    TEST_ASSERT_EQUAL_INT(1, callback_count.load());
}

//! @test Test concurrent callback access
void test_concurrent_callback_access(void) {
    //! @section Arrange
    std::atomic<int> callback_count{0};
    NativeBleDriver driver(DeviceId(0x12345678));
    driver.begin();

    driver.set_reading_callback([&callback_count](DeviceId, const ReadingMsg&) {
        callback_count++;
    });

    std::vector<BlePayload> payloads;
    for (int i = 0; i < 10; ++i) {
        ReadingMsg reading_msg;
        reading_msg.sender_id = DeviceId(0x87654321);
        reading_msg.session_id = SessionId(0x11111111);
        reading_msg.offset_ms = i * 1000;
        reading_msg.temperature_c_centi = 2500 + i;
        reading_msg.humidity_bp = 5000 + i;

        BlePayload payload;
        ReadingMsg::serialize(reading_msg, payload);
        payloads.push_back(std::move(payload));
    }

    //! @section Act - Send all messages
    for (auto& payload : payloads) {
        driver.send_to(DeviceId(0x12345678), std::move(payload));
    }

    //! @section Assert - All callbacks should be invoked
    TEST_ASSERT_EQUAL_INT(10, callback_count.load());
}

