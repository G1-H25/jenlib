//! @file tests/BleIntegrationTests.cpp
//! @brief Integration tests for complete BLE callback message flow.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)
//!
//! Tests cover:
//! - Complete sensor-broker communication flow
//! - End-to-end callback message routing
//! - Real-world usage scenarios
//! - Performance and reliability

#include <unity.h>
#include <cstdint>
#include <atomic>
#include <chrono>
#include <memory>
#include <vector>
#include "jenlib/ble/BleDriver.h"
#include "jenlib/ble/Ble.h"
#include "jenlib/ble/Messages.h"
#include "jenlib/ble/drivers/NativeBleDriver.h"

using jenlib::ble::BLE;
using jenlib::ble::BleDriver;
using jenlib::ble::DeviceId;
using jenlib::ble::ReadingMsg;
using jenlib::ble::ReceiptMsg;
using jenlib::ble::SessionId;
using jenlib::ble::StartBroadcastMsg;

//! @brief Test helper: Complete sensor simulation
class SensorSimulator {
 public:
    explicit SensorSimulator(DeviceId sensor_id, std::shared_ptr<BleDriver> driver)
        : sensor_id_(sensor_id), driver_(driver), session_active_(false), reading_count_(0) {
        // Set up type-specific callbacks
        driver_->set_start_broadcast_callback([this](DeviceId sender_id, const StartBroadcastMsg& msg) {
            on_start_broadcast(sender_id, msg);
        });
        driver_->set_receipt_callback([this](DeviceId sender_id, const ReceiptMsg& msg) {
            on_receipt(sender_id, msg);
        });
    }

    void start_session(SessionId session_id) {
        current_session_id_ = session_id;
        session_active_ = true;
        reading_count_ = 0;
    }

    void stop_session() {
        session_active_ = false;
    }

    void send_reading() {
        if (!session_active_) return;

        ReadingMsg reading;
        reading.sender_id = sensor_id_;
        reading.session_id = current_session_id_;
        reading.offset_ms = reading_count_ * 1000;  //  1 second intervals
        reading.temperature_c_centi = 2300 + reading_count_ * 10;  //  Simulate temperature
        reading.humidity_bp = 5000 + reading_count_ * 50;  //  Simulate humidity

        BLE::broadcast_reading(sensor_id_, reading);
        reading_count_++;
    }

    bool is_session_active() const { return session_active_; }
    int get_reading_count() const { return reading_count_; }
    SessionId get_current_session() const { return current_session_id_; }

 private:
    void on_start_broadcast(DeviceId sender_id, const StartBroadcastMsg& msg) {
        if (msg.device_id.value() == sensor_id_.value()) {
            start_session(msg.session_id);
        }
    }

    void on_receipt(DeviceId sender_id, const ReceiptMsg& msg) {
        if (msg.session_id.value() == current_session_id_.value()) {
            // Acknowledge receipt (in real implementation, would purge buffered readings)
        }
    }

    DeviceId sensor_id_;
    std::shared_ptr<BleDriver> driver_;
    SessionId current_session_id_;
    bool session_active_;
    int reading_count_;
};

//! @brief Test helper: Complete broker simulation
class BrokerSimulator {
 public:
    explicit BrokerSimulator(std::shared_ptr<BleDriver> driver)
        : driver_(driver), session_active_(false), total_readings_(0) {
        // Set up type-specific callback for readings
        driver_->set_reading_callback([this](DeviceId sender_id, const ReadingMsg& msg) {
            on_reading(sender_id, msg);
        });
    }

    void start_session(DeviceId target_sensor, SessionId session_id) {
        StartBroadcastMsg start_msg;
        start_msg.device_id = target_sensor;
        start_msg.session_id = session_id;

        BLE::send_start(target_sensor, start_msg);
        session_active_ = true;
        current_session_id_ = session_id;
        total_readings_ = 0;
    }

    void send_receipt(DeviceId target_sensor, uint32_t up_to_offset_ms) {
        ReceiptMsg receipt;
        receipt.session_id = current_session_id_;
        receipt.up_to_offset_ms = up_to_offset_ms;

        BLE::send_receipt(target_sensor, receipt);
    }

    bool is_session_active() const { return session_active_; }
    int get_total_readings() const { return total_readings_; }
    SessionId get_current_session() const { return current_session_id_; }

 private:
    void on_reading(DeviceId sender_id, const ReadingMsg& msg) {
        total_readings_++;

        // Send receipt every 5 readings
        if (total_readings_ % 5 == 0) {
            send_receipt(sender_id, msg.offset_ms);
        }
    }

    std::shared_ptr<BleDriver> driver_;
    SessionId current_session_id_;
    bool session_active_;
    int total_readings_;
};

//! @test Test complete sensor-broker communication flow
void test_complete_sensor_broker_communication_flow(void) {
    // Arrange
    auto driver = std::make_shared<NativeBleDriver>(DeviceId(0x00000000));  // Broker
    BLE::set_driver(driver.get());
    driver->begin();

    DeviceId sensor_id(0x12345678);
    SessionId session_id(0x87654321);

    SensorSimulator sensor(sensor_id, driver);
    BrokerSimulator broker(driver);

    // Act - Start session
    broker.start_session(sensor_id, session_id);

    // Send multiple readings
    for (int i = 0; i < 10; ++i) {
        sensor.send_reading();
        driver->poll();  //  Process BLE events
    }

    // Assert - Verify complete flow
    TEST_ASSERT_TRUE(sensor.is_session_active());
    TEST_ASSERT_EQUAL_INT(10, sensor.get_reading_count());
    TEST_ASSERT_TRUE(broker.is_session_active());
    TEST_ASSERT_EQUAL_INT(10, broker.get_total_readings());
    TEST_ASSERT_EQUAL_UINT32(session_id.value(), sensor.get_current_session().value());
    TEST_ASSERT_EQUAL_UINT32(session_id.value(), broker.get_current_session().value());
}

//! @test Test multiple sensors with single broker
void test_multiple_sensors_single_broker(void) {
    // Arrange
    auto driver = std::make_shared<NativeBleDriver>(DeviceId(0x00000000));  // Broker
    BLE::set_driver(driver.get());
    driver->begin();

    std::vector<DeviceId> sensor_ids = {
        DeviceId(0x11111111),
        DeviceId(0x22222222),
        DeviceId(0x33333333)
    };

    std::vector<SensorSimulator> sensors;
    for (const auto& sensor_id : sensor_ids) {
        sensors.emplace_back(sensor_id, driver);
    }

    BrokerSimulator broker(driver);

    // Act - Start sessions for all sensors
    for (size_t i = 0; i < sensor_ids.size(); ++i) {
        SessionId session_id(0x10000000 + i);
        broker.start_session(sensor_ids[i], session_id);
    }

    // Send readings from all sensors
    for (int reading = 0; reading < 5; ++reading) {
        for (auto& sensor : sensors) {
            sensor.send_reading();
        }
        driver->poll();  //  Process BLE events
    }

    // Assert - Verify all sensors are active and have sent readings
    for (const auto& sensor : sensors) {
        TEST_ASSERT_TRUE(sensor.is_session_active());
        TEST_ASSERT_EQUAL_INT(5, sensor.get_reading_count());
    }

    TEST_ASSERT_TRUE(broker.is_session_active());
    TEST_ASSERT_EQUAL_INT(15, broker.get_total_readings());  //  3 sensors * 5 readings
}

//! @test Test session management and cleanup
void test_session_management_and_cleanup(void) {
    // Arrange
    auto driver = std::make_shared<NativeBleDriver>(DeviceId(0x00000000));
    BLE::set_driver(driver.get());
    driver->begin();

    DeviceId sensor_id(0x12345678);
    SensorSimulator sensor(sensor_id, driver);
    BrokerSimulator broker(driver);

    // Act - Start first session
    SessionId session1(0x11111111);
    broker.start_session(sensor_id, session1);

    // Send some readings
    for (int i = 0; i < 3; ++i) {
        sensor.send_reading();
        driver->poll();
    }

    // Start second session (should replace first)
    SessionId session2(0x22222222);
    broker.start_session(sensor_id, session2);

    // Send more readings
    for (int i = 0; i < 3; ++i) {
        sensor.send_reading();
        driver->poll();
    }

    // Assert - Verify session management
    TEST_ASSERT_TRUE(sensor.is_session_active());
    TEST_ASSERT_EQUAL_INT(6, sensor.get_reading_count());  //  Total readings
    TEST_ASSERT_EQUAL_UINT32(session2.value(), sensor.get_current_session().value());  //  Latest session
    TEST_ASSERT_TRUE(broker.is_session_active());
}

//! @test Test callback performance under load
void test_callback_performance_under_load(void) {
    // Arrange
    auto driver = std::make_shared<NativeBleDriver>(DeviceId(0x00000000));
    BLE::set_driver(driver.get());
    driver->begin();

    std::atomic<int> callback_count{0};
    driver->set_reading_callback([&callback_count](DeviceId, const ReadingMsg&) {
        callback_count++;
    });

    DeviceId sensor_id(0x12345678);
    SessionId session_id(0x87654321);

    // Act - Send many messages rapidly
    const int message_count = 1000;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < message_count; ++i) {
        ReadingMsg reading;
        reading.sender_id = sensor_id;
        reading.session_id = session_id;
        reading.offset_ms = i;
        reading.temperature_c_centi = 2500;
        reading.humidity_bp = 5000;

        BLE::broadcast_reading(sensor_id, reading);

        if (i % 100 == 0) {
            driver->poll();  //  Process events periodically
        }
    }

    // Process remaining events
    driver->poll();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Assert - Verify performance
    TEST_ASSERT_EQUAL_INT(message_count, callback_count.load());
    TEST_ASSERT_TRUE(duration.count() < 1000);  //  Should complete within 1 second
}

//! @test Test callback reliability with message loss simulation
void test_callback_reliability_with_message_loss(void) {
    // Arrange
    auto driver = std::make_shared<NativeBleDriver>(DeviceId(0x00000000));
    BLE::set_driver(driver.get());
    driver->begin();

    std::atomic<int> callback_count{0};
    driver->set_reading_callback([&callback_count](DeviceId, const ReadingMsg&) {
        callback_count++;
    });

    DeviceId sensor_id(0x12345678);
    SessionId session_id(0x87654321);

    // Act - Send messages with simulated loss (skip some messages)
    const int total_messages = 100;
    const int sent_messages = 75;  //  Simulate 25% loss

    for (int i = 0; i < total_messages; ++i) {
        if (i % 4 != 0) {  //  Skip every 4th message (simulate loss)
            ReadingMsg reading;
            reading.sender_id = sensor_id;
            reading.session_id = session_id;
            reading.offset_ms = i;
            reading.temperature_c_centi = 2500;
            reading.humidity_bp = 5000;

            BLE::broadcast_reading(sensor_id, reading);
        }

        if (i % 10 == 0) {
            driver->poll();
        }
    }

    driver->poll();

    // Assert - Verify reliability
    TEST_ASSERT_EQUAL_INT(sent_messages, callback_count.load());
    TEST_ASSERT_TRUE(callback_count.load() > 0);  //  At least some messages should be received
}

//! @test Test callback error recovery
void test_callback_error_recovery(void) {
    // Arrange
    auto driver = std::make_shared<NativeBleDriver>(DeviceId(0x00000000));
    BLE::set_driver(driver.get());
    driver->begin();

    std::atomic<int> callback_count{0};
    std::atomic<int> error_count{0};

    // Register callback that sometimes throws
    driver->set_reading_callback([&callback_count, &error_count](DeviceId, const ReadingMsg&) {
        callback_count++;
        if (callback_count.load() % 3 == 0) {
            error_count++;
            throw std::runtime_error("Simulated callback error");
        }
    });

    DeviceId sensor_id(0x12345678);
    SessionId session_id(0x87654321);

    // Act - Send messages (some will cause callback errors)
    const int message_count = 15;
    for (int i = 0; i < message_count; ++i) {
        ReadingMsg reading;
        reading.sender_id = sensor_id;
        reading.session_id = session_id;
        reading.offset_ms = i;
        reading.temperature_c_centi = 2500;
        reading.humidity_bp = 5000;

        try {
            BLE::broadcast_reading(sensor_id, reading);
        } catch (...) {
            // Errors should be handled gracefully
        }

        if (i % 5 == 0) {
            driver->poll();
        }
    }

    driver->poll();

    // Assert - Verify error recovery
    TEST_ASSERT_EQUAL_INT(message_count, callback_count.load());
    TEST_ASSERT_EQUAL_INT(5, error_count.load());  //  Every 3rd message should cause error
}

//! @test Test callback with mixed message types
void test_callback_with_mixed_message_types(void) {
    // Arrange
    auto driver = std::make_shared<NativeBleDriver>(DeviceId(0x00000000));
    BLE::set_driver(driver.get());
    driver->begin();

    std::atomic<int> start_broadcast_count{0};
    std::atomic<int> reading_count{0};
    std::atomic<int> receipt_count{0};

    // Register all type-specific callbacks
    driver->set_start_broadcast_callback([&start_broadcast_count](DeviceId, const StartBroadcastMsg&) {
        start_broadcast_count++;
    });

    driver->set_reading_callback([&reading_count](DeviceId, const ReadingMsg&) {
        reading_count++;
    });

    driver->set_receipt_callback([&receipt_count](DeviceId, const ReceiptMsg&) {
        receipt_count++;
    });

    DeviceId sensor_id(0x12345678);
    SessionId session_id(0x87654321);

    // Act - Send mixed message types
    // Start broadcast
    StartBroadcastMsg start_msg;
    start_msg.device_id = sensor_id;
    start_msg.session_id = session_id;
    BLE::send_start(sensor_id, start_msg);

    // Reading messages
    for (int i = 0; i < 3; ++i) {
        ReadingMsg reading;
        reading.sender_id = sensor_id;
        reading.session_id = session_id;
        reading.offset_ms = i * 1000;
        reading.temperature_c_centi = 2500;
        reading.humidity_bp = 5000;
        BLE::broadcast_reading(sensor_id, reading);
    }

    // Receipt message
    ReceiptMsg receipt;
    receipt.session_id = session_id;
    receipt.up_to_offset_ms = 2000;
    BLE::send_receipt(sensor_id, receipt);

    driver->poll();

    // Assert - Verify all message types were handled
    TEST_ASSERT_EQUAL_INT(1, start_broadcast_count.load());
    TEST_ASSERT_EQUAL_INT(3, reading_count.load());
    TEST_ASSERT_EQUAL_INT(1, receipt_count.load());
}

//! @test Test callback with concurrent access
void test_callback_with_concurrent_access(void) {
    // Arrange
    auto driver = std::make_shared<NativeBleDriver>(DeviceId(0x00000000));
    BLE::set_driver(driver.get());
    driver->begin();

    std::atomic<int> callback_count{0};
    driver->set_reading_callback([&callback_count](DeviceId, const ReadingMsg&) {
        callback_count++;
    });

    DeviceId sensor_id(0x12345678);
    SessionId session_id(0x87654321);

    // Act - Send messages from multiple "threads" (simulated)
    const int messages_per_thread = 50;
    const int thread_count = 4;

    for (int thread = 0; thread < thread_count; ++thread) {
        for (int i = 0; i < messages_per_thread; ++i) {
            ReadingMsg reading;
            reading.sender_id = DeviceId(sensor_id.value() + thread);  //  Different sender IDs
            reading.session_id = session_id;
            reading.offset_ms = thread * messages_per_thread + i;
            reading.temperature_c_centi = 2500;
            reading.humidity_bp = 5000;

            BLE::broadcast_reading(reading.sender_id, reading);
        }
    }

    driver->poll();

    // Assert - Verify all messages were processed
    TEST_ASSERT_EQUAL_INT(thread_count * messages_per_thread, callback_count.load());
}

