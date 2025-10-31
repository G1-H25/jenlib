//! @file include/jenlib/ble/drivers/EspIdfBleDriver.h
//! @brief ESP-IDF BLE driver interface.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_BLE_DRIVERS_ESPIDFBLEDRIVER_H_
#define INCLUDE_JENLIB_BLE_DRIVERS_ESPIDFBLEDRIVER_H_

#include <jenlib/ble/BleDriver.h>
#include <jenlib/ble/PayloadBuffer.h>
#include <string>

#ifdef ESP_PLATFORM
#include <esp_gattc_api.h>
#include <esp_gatts_api.h>
#include <esp_gap_ble_api.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gap_bt_api.h>
#include <esp_gatt_common_api.h>
#include <queue>

namespace jenlib::ble {

//! @brief ESP-IDF BLE driver implementation using ESP32 BLE stack.
//!
//! @note Precondition: The application must initialize NVS (nvs_flash_init) once
//! at startup before constructing this driver or calling begin(). NVS is a
//! system-wide resource and should not be initialized by the driver.
class EspIdfBleDriver : public BleDriver {
 public:
    //! @brief Constructor.
    EspIdfBleDriver(const std::string& device_name, DeviceId local_device_id);

    //! @brief Constructor with callbacks.
    EspIdfBleDriver(const std::string& device_name, DeviceId local_device_id,
                    const BleCallbacks& callbacks);

    //! @brief Destructor.
    ~EspIdfBleDriver() override;

    //! @brief Initialize BLE stack and start advertising.
    //! @note Requires NVS to be initialized by the application beforehand.
    bool begin() override;

    //! @brief Stop BLE stack.
    void end() override;

    //! @brief Advertise data to connected devices.
    void advertise(DeviceId device_id, BlePayload payload) override;

    //! @brief Send data to specific device.
    void send_to(DeviceId device_id, BlePayload payload) override;

    //! @brief Receive data from any device.
    bool receive(DeviceId self_id, BlePayload& out_payload) override;

    //! @brief Poll for BLE events.
    void poll() override;

    //! @brief Set generic message callback.
    void set_message_callback(BleMessageCallback callback) override;

    //! @brief Clear generic message callback.
    void clear_message_callback() override;

    //! @brief Set start broadcast callback.
    void set_start_broadcast_callback(StartBroadcastCallback callback) override;

    //! @brief Set reading callback.
    void set_reading_callback(ReadingCallback callback) override;

    //! @brief Set receipt callback.
    void set_receipt_callback(ReceiptCallback callback) override;

    //! @brief Clear all type-specific callbacks.
    void clear_type_specific_callbacks() override;

    //! @brief Set connection callback.
    void set_connection_callback(ConnectionCallback callback) override;

    //! @brief Clear connection callback.
    void clear_connection_callback() override;

    //! @brief Check if connected to any device.
    bool is_connected() const override;

 private:
    // Active instance used by static ESP-IDF callbacks to route events
    static EspIdfBleDriver* instance_;

    std::string device_name_;                    //!< Device name for advertising
    DeviceId local_device_id_;                  //!< Local device ID
    bool initialized_;                          //!< Initialization state
    bool last_connected_state_;                 //!< Last connection state

    // Callbacks
    BleMessageCallback message_callback_;
    StartBroadcastCallback start_broadcast_callback_;
    ReadingCallback reading_callback_;
    ReceiptCallback receipt_callback_;
    ConnectionCallback connection_callback_;

    // ESP-IDF BLE specific
    uint16_t gatts_if_;                         //!< GATT server interface
    uint16_t conn_id_;                          //!< Connection ID
    uint16_t service_handle_;                   //!< Service handle
    uint16_t control_char_handle_;               //!< Control characteristic handle
    uint16_t reading_char_handle_;              //!< Reading characteristic handle
    uint16_t receipt_char_handle_;              //!< Receipt characteristic handle

    // Payload buffer for received data
    PayloadBuffer received_payloads_;

    //! @brief Setup GATT service and characteristics.
    void setup_gatt_service();

    //! @brief Process BLE events from ESP-IDF stack.
    void process_ble_events();

    //! @brief Handle GATT server events.
    static void gatts_event_handler(esp_gatts_cb_event_t event,
                                    esp_gatt_if_t gatts_if,
                                    esp_ble_gatts_cb_param_t* param);

    //! @brief Handle GAP events.
    static void gap_event_handler(esp_gap_ble_cb_event_t event,
                                  esp_ble_gap_cb_param_t* param);

    //! @brief Queue received payload.
    void queue_received_payload(BlePayload payload);

    //! @brief Check if payload is pending for device.
    bool has_pending_payload(DeviceId device_id) const;

    //! @brief Get pending payload for device.
    bool get_pending_payload(DeviceId device_id, BlePayload& out_payload);

    //! @brief Extract sender ID from connection.
    DeviceId extract_sender_id_from_connection();

    //! @brief Try type-specific callbacks.
    bool try_type_specific_callbacks(DeviceId sender_id, const BlePayload& payload);

    //! @brief Send via GATT characteristic.
    void send_via_gatt(uint16_t char_handle, const BlePayload& payload);

    //! @brief Send via advertising data.
    void send_via_advertising(const BlePayload& payload);
};

}  // namespace jenlib::ble

#else
// Fallback implementation for non-ESP platforms
namespace jenlib::ble {

//! @brief Empty stub implementation for non-ESP platforms.
class EspIdfBleDriver {
 public:
    EspIdfBleDriver() = delete;
};

}  // namespace jenlib::ble

#endif  // ESP_PLATFORM

#endif  // INCLUDE_JENLIB_BLE_DRIVERS_ESPIDFBLEDRIVER_H_
