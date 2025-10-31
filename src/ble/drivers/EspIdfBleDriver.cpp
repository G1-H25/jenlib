//! @file src/ble/drivers/EspIdfBleDriver.cpp
//! @brief ESP-IDF BLE driver implementation using ESP32 BLE stack.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <jenlib/ble/drivers/EspIdfBleDriver.h>
#include <jenlib/ble/Messages.h>
#include <jenlib/ble/GattProfile.h>
#include <utility>
#include <string_view>

#ifdef ESP_PLATFORM
#include <esp_gattc_api.h>
#include <esp_gatts_api.h>
#include <esp_gap_ble_api.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gap_bt_api.h>
#include <esp_gatt_common_api.h>
#include <string.h>

namespace jenlib::ble {

// Static instance used for routing static callbacks to the driver object
EspIdfBleDriver* EspIdfBleDriver::instance_ = nullptr;

EspIdfBleDriver::EspIdfBleDriver(const std::string& device_name, DeviceId local_device_id)
    : device_name_(device_name), local_device_id_(local_device_id) {
    message_callback_ = nullptr;
    start_broadcast_callback_ = nullptr;
    reading_callback_ = nullptr;
    receipt_callback_ = nullptr;
    connection_callback_ = nullptr;
    initialized_ = false;
    last_connected_state_ = false;
    gatts_if_ = ESP_GATT_IF_NONE;
    conn_id_ = 0;
    service_handle_ = 0;
    control_char_handle_ = 0;
    reading_char_handle_ = 0;
    receipt_char_handle_ = 0;
}

EspIdfBleDriver::EspIdfBleDriver(const std::string& device_name, DeviceId local_device_id,
                                  const BleCallbacks& callbacks)
    : EspIdfBleDriver(device_name, local_device_id) {
    if (callbacks.on_connection) set_connection_callback(callbacks.on_connection);
    if (callbacks.on_start) set_start_broadcast_callback(callbacks.on_start);
    if (callbacks.on_reading) set_reading_callback(callbacks.on_reading);
    if (callbacks.on_receipt) set_receipt_callback(callbacks.on_receipt);
    if (callbacks.on_generic) set_message_callback(callbacks.on_generic);
}

EspIdfBleDriver::~EspIdfBleDriver() {
    end();
}

bool EspIdfBleDriver::begin() {
    if (initialized_) {
        return true;
    }

    // NVS must have been initialized by the application prior to calling begin()

    // Initialize BT controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_err_t ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        return false;
    }

    // Enable BT controller
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        return false;
    }

    // Initialize Bluedroid
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        return false;
    }

    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        return false;
    }

    // Make this instance active for static callback routing BEFORE registration
    instance_ = this;

    // Register GATT server callback
    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret != ESP_OK) {
        return false;
    }

    // Register GAP callback
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret != ESP_OK) {
        return false;
    }

    // Setup GATT service
    setup_gatt_service();

    // Start advertising
    esp_ble_adv_data_t adv_data = {};
    adv_data.set_scan_rsp = false;
    adv_data.include_name = true;
    adv_data.include_txpower = true;
    adv_data.min_interval = 0x0006;  // slave connection min interval
    adv_data.max_interval = 0x0010;  // slave connection max interval
    adv_data.appearance = 0x00;
    adv_data.manufacturer_len = 0;
    adv_data.p_manufacturer_data = nullptr;
    adv_data.service_data_len = 0;
    adv_data.p_service_data = nullptr;
    adv_data.service_uuid_len = 0;
    adv_data.p_service_uuid = nullptr;
    adv_data.flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);

    esp_ble_gap_config_adv_data(&adv_data);

    // Advertising will be started from GAP callback once data is set

    initialized_ = true;
    return true;
}

void EspIdfBleDriver::end() {
    if (initialized_) {
        esp_ble_gap_stop_advertising();
        esp_bluedroid_disable();
        esp_bluedroid_deinit();
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
        initialized_ = false;
    }
}

void EspIdfBleDriver::advertise(DeviceId device_id, BlePayload payload) {
    if (!initialized_) {
        return;
    }

    // Use reading characteristic to broadcast data
    if (is_connected()) {
        send_via_gatt(reading_char_handle_, payload);
    }
}

void EspIdfBleDriver::send_to(DeviceId device_id, BlePayload payload) {
    // For sensor role, this is typically not used
    // Directed sends are handled by the broker
    (void)device_id;
    (void)payload;
}

bool EspIdfBleDriver::receive(DeviceId self_id, BlePayload& out_payload) {
    if (!initialized_) {
        return false;
    }

    // Process BLE events to handle incoming data
    process_ble_events();

    // Check for pending payloads
    return get_pending_payload(self_id, out_payload);
}

void EspIdfBleDriver::poll() {
    if (!initialized_) {
        return;
    }

    // Process BLE events
    process_ble_events();
}

void EspIdfBleDriver::set_message_callback(BleMessageCallback callback) {
    message_callback_ = std::move(callback);
}

void EspIdfBleDriver::clear_message_callback() {
    message_callback_ = nullptr;
}

void EspIdfBleDriver::set_start_broadcast_callback(StartBroadcastCallback callback) {
    start_broadcast_callback_ = std::move(callback);
}

void EspIdfBleDriver::set_reading_callback(ReadingCallback callback) {
    reading_callback_ = std::move(callback);
}

void EspIdfBleDriver::set_receipt_callback(ReceiptCallback callback) {
    receipt_callback_ = std::move(callback);
}

void EspIdfBleDriver::clear_type_specific_callbacks() {
    start_broadcast_callback_ = nullptr;
    reading_callback_ = nullptr;
    receipt_callback_ = nullptr;
}

void EspIdfBleDriver::set_connection_callback(ConnectionCallback callback) {
    connection_callback_ = std::move(callback);
}

void EspIdfBleDriver::clear_connection_callback() {
    connection_callback_ = nullptr;
}

bool EspIdfBleDriver::is_connected() const {
    return initialized_ && (conn_id_ != 0);
}

void EspIdfBleDriver::setup_gatt_service() {
    // Create service
    esp_gatt_srvc_id_t service_id = {};
    service_id.is_primary = true;
    service_id.id.inst_id = 0x00;
    service_id.id.uuid.len = ESP_UUID_LEN_128;
    memcpy(service_id.id.uuid.uuid.uuid128, jenlib::ble::gatt::kServiceSensor, 16);

    esp_ble_gatts_create_service(gatts_if_, &service_id, 3);  // 3 characteristics
}

void EspIdfBleDriver::process_ble_events() {
    // ESP-IDF handles events through callbacks
    // This method is kept for compatibility with the interface
}

void EspIdfBleDriver::gatts_event_handler(esp_gatts_cb_event_t event,
                                          esp_gatt_if_t gatts_if,
                                          esp_ble_gatts_cb_param_t* param) {
    EspIdfBleDriver* self = instance_;
    if (self == nullptr) {
        return;
    }

    switch (event) {
        case ESP_GATTS_REG_EVT: {
            self->gatts_if_ = gatts_if;
            // Begin service creation (UUIDs are configured in setup_gatt_service)
            self->setup_gatt_service();
            break;
        }
        case ESP_GATTS_CREATE_EVT: {
            if (param) {
                self->service_handle_ = param->create.service_handle;
                // Add characteristics: control (write), reading (notify), receipt (write)
                esp_bt_uuid_t chr_uuid = {};

                // Control characteristic
                chr_uuid.len = ESP_UUID_LEN_128;
                memcpy(chr_uuid.uuid.uuid128, jenlib::ble::gatt::kChrControl, sizeof(chr_uuid.uuid.uuid128));
                esp_gatt_char_prop_t control_props = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
                esp_ble_gatts_add_char(self->service_handle_, &chr_uuid,
                                       ESP_GATT_PERM_WRITE,
                                       control_props, nullptr, nullptr);

                // Reading characteristic
                chr_uuid.len = ESP_UUID_LEN_128;
                memcpy(chr_uuid.uuid.uuid128, jenlib::ble::gatt::kChrReading, sizeof(chr_uuid.uuid.uuid128));
                esp_gatt_char_prop_t reading_props = ESP_GATT_CHAR_PROP_BIT_NOTIFY | ESP_GATT_CHAR_PROP_BIT_INDICATE;
                esp_ble_gatts_add_char(self->service_handle_, &chr_uuid,
                                       ESP_GATT_PERM_READ,
                                       reading_props, nullptr, nullptr);

                // Receipt characteristic
                chr_uuid.len = ESP_UUID_LEN_128;
                memcpy(chr_uuid.uuid.uuid128, jenlib::ble::gatt::kChrReceipt, sizeof(chr_uuid.uuid.uuid128));
                esp_gatt_char_prop_t receipt_props = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
                esp_ble_gatts_add_char(self->service_handle_, &chr_uuid,
                                       ESP_GATT_PERM_WRITE,
                                       receipt_props, nullptr, nullptr);
            }
            break;
        }
        case ESP_GATTS_ADD_CHAR_EVT: {
            if (param) {
                // Track handles for our known characteristics by comparing UUIDs
                const esp_bt_uuid_t& uuid = param->add_char.char_uuid;
                if (uuid.len == ESP_UUID_LEN_128) {
                    if (memcmp(uuid.uuid.uuid128, jenlib::ble::gatt::kChrControl, 16) == 0) {
                        self->control_char_handle_ = param->add_char.attr_handle;
                    } else if (memcmp(uuid.uuid.uuid128, jenlib::ble::gatt::kChrReading, 16) == 0) {
                        self->reading_char_handle_ = param->add_char.attr_handle;
                    } else if (memcmp(uuid.uuid.uuid128, jenlib::ble::gatt::kChrReceipt, 16) == 0) {
                        self->receipt_char_handle_ = param->add_char.attr_handle;
                    }
                }
            }
            break;
        }
        case ESP_GATTS_WRITE_EVT: {
            if (param && param->write.is_prep == false) {
                const uint16_t handle = param->write.handle;
                const uint8_t* data = param->write.value;
                const uint16_t len = param->write.len;

                if ((handle == self->control_char_handle_) || (handle == self->receipt_char_handle_)) {
                    BlePayload payload;
                    payload.append_raw(data, len);

                    const DeviceId sender_id = self->extract_sender_id_from_connection();

                    // Route to type-specific first, then generic
                    if (!self->try_type_specific_callbacks(sender_id, payload)) {
                        if (self->message_callback_) {
                            self->message_callback_(sender_id, payload);
                        }
                    }
                }
            }
            break;
        }
        case ESP_GATTS_READ_EVT: {
            // Optionally handle static reads if needed in future
            (void)param;
            break;
        }
        case ESP_GATTS_CONNECT_EVT: {
            if (param) {
                self->conn_id_ = param->connect.conn_id;
                self->last_connected_state_ = true;
                if (self->connection_callback_) {
                    self->connection_callback_(true);
                }
            }
            break;
        }
        case ESP_GATTS_DISCONNECT_EVT: {
            self->last_connected_state_ = false;
            self->conn_id_ = 0;
            if (self->connection_callback_) {
                self->connection_callback_(false);
            }
            break;
        }
        case ESP_GATTS_MTU_EVT:
        case ESP_GATTS_CONF_EVT:
        case ESP_GATTS_START_EVT:
        default:
            break;
    }
}

void EspIdfBleDriver::gap_event_handler(esp_gap_ble_cb_event_t event,
                                        esp_ble_gap_cb_param_t* param) {
    EspIdfBleDriver* self = instance_;
    if (self == nullptr) {
        return;
    }

    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT: {
            // Start advertising after adv data configured
            esp_ble_adv_params_t adv_params = {};
            adv_params.adv_int_min = 0x20;
            adv_params.adv_int_max = 0x40;
            adv_params.adv_type = ADV_TYPE_IND;
            adv_params.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
            adv_params.channel_map = ADV_CHNL_ALL;
            adv_params.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;
            esp_ble_gap_start_advertising(&adv_params);
            (void)param;
            break;
        }
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT: {
            (void)param;
            break;
        }
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT: {
            (void)param;
            break;
        }
        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            (void)param;
            break;
        case ESP_GAP_BLE_AUTH_CMPL_EVT:
            (void)param;
            break;
        default:
            break;
    }
}

void EspIdfBleDriver::queue_received_payload(BlePayload payload) {
    received_payloads_.push(std::move(payload));
}

bool EspIdfBleDriver::has_pending_payload(DeviceId device_id) const {
    (void)device_id;
    return !received_payloads_.empty();
}

bool EspIdfBleDriver::get_pending_payload(DeviceId device_id, BlePayload& out_payload) {
    (void)device_id;
    return received_payloads_.pop(out_payload);
}

DeviceId EspIdfBleDriver::extract_sender_id_from_connection() {
    // In a real implementation, extract sender ID from connection
    return DeviceId(0x00000000);  // Placeholder
}

bool EspIdfBleDriver::try_type_specific_callbacks(DeviceId sender_id, const BlePayload& payload) {
    // Try StartBroadcastMsg
    if (start_broadcast_callback_) {
        StartBroadcastMsg start_msg;
        if (StartBroadcastMsg::deserialize(payload, start_msg)) {
            start_broadcast_callback_(sender_id, start_msg);
            return true;
        }
    }

    // Try ReadingMsg
    if (reading_callback_) {
        ReadingMsg reading;
        if (ReadingMsg::deserialize(payload, reading)) {
            reading_callback_(sender_id, reading);
            return true;
        }
    }

    // Try ReceiptMsg
    if (receipt_callback_) {
        ReceiptMsg receipt;
        if (ReceiptMsg::deserialize(payload, receipt)) {
            receipt_callback_(sender_id, receipt);
            return true;
        }
    }

    return false;
}

void EspIdfBleDriver::send_via_gatt(uint16_t char_handle, const BlePayload& payload) {
    // esp_ble_gatts_send_indicate expects a non-const uint8_t* for the value
    // Cast is safe here as the API does not modify the buffer
    esp_ble_gatts_send_indicate(gatts_if_, conn_id_, char_handle,
                                payload.size,
                                const_cast<uint8_t*>(payload.bytes.data()),
                                false);
}

void EspIdfBleDriver::send_via_advertising(const BlePayload& payload) {
    // Implementation would set advertising data
    (void)payload;
}

}  // namespace jenlib::ble

#else
// Empty implementation for non-ESP platforms
// The header file already provides deleted constructors
namespace jenlib::ble {
    // No implementation needed - constructors are deleted in header
}

#endif  // ESP_PLATFORM
