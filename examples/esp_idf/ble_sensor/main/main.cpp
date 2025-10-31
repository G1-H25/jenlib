//! @file examples/esp_idf/ble_sensor/main/main.cpp
//! @brief BLE sensor example for ESP-IDF, using state machine and event driven architecture.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <jenlib/ble/Roles.h>
#include <jenlib/ble/Ids.h>
#include <jenlib/ble/Messages.h>
#include <jenlib/events/EventDispatcher.h>
#include <jenlib/time/Time.h>
#include <jenlib/state/SensorStateMachine.h>
#include <jenlib/measurement/Measurement.h>
#include <jenlib/gpio/drivers/EspIdfGpioDriver.h>
#include <jenlib/ble/drivers/EspIdfBleDriver.h>
#include <jenlib/time/drivers/EspIdfTimeDriver.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_system.h>

static const char* TAG = "jenlib_sensor";

//! @section Global state
jenlib::events::EventDispatcher event_dispatcher;  //!< Event dispatcher
constexpr jenlib::ble::DeviceId kDeviceId = jenlib::ble::DeviceId(0x12345678);  //!< Some DeviceID
static jenlib::ble::Sensor sensor(kDeviceId);  //!< We are a Sensor

//! @section ESP-IDF driver construction
static jenlib::gpio::EspIdfGpioDriver gpio_driver;
static jenlib::ble::EspIdfBleDriver ble_driver("JenLibSensor", kDeviceId);
static jenlib::time::EspIdfTimeDriver time_driver;

//! @section State machine
jenlib::state::SensorStateMachine sensor_state_machine;

//! @section Forward declaration of functions
//! BLE event callback functions
void callback_connection(bool connected);
void callback_start(jenlib::ble::DeviceId sender_id, const jenlib::ble::StartBroadcastMsg &msg);
void callback_receipt(jenlib::ble::DeviceId sender_id, const jenlib::ble::ReceiptMsg &msg);
void callback_generic(jenlib::ble::DeviceId sender_id, const jenlib::ble::BlePayload &payload);

//! Event handlers
void handle_measurement_timer();
void handle_ble_message_event(const jenlib::events::Event& event);
void handle_connection_state_event(const jenlib::events::Event& event);
void handle_time_tick_event(const jenlib::events::Event& event);

//! Helper functions
void start_measurement_session(const jenlib::ble::StartBroadcastMsg& msg);
void stop_measurement_session();
void take_and_broadcast_reading();
float read_temperature_sensor();  // Mock sensor reading
float read_humidity_sensor();     // Mock sensor reading

//! @section Main task
void sensor_task(void* pvParameters) {
    ESP_LOGI(TAG, "Starting sensor task");

    // Initialize time service first
    jenlib::time::Time::initialize();

    // Initialize BLE communication
    if (!sensor.begin()) {
        ESP_LOGE(TAG, "Failed to initialize BLE sensor");
        vTaskDelete(nullptr);
        return;
    }

    sensor.configure_callbacks(jenlib::ble::BleCallbacks{
        .on_connection = callback_connection,
        .on_start = callback_start,
        .on_receipt = callback_receipt,
        .on_generic = callback_generic,
    });

    // Configure state machine callbacks for debugging/monitoring
    sensor_state_machine.set_state_action_callback(
        [](jenlib::state::StateAction action,
           jenlib::state::SensorState state) {
            ESP_LOGI(TAG, "State action: %d, State: %d",
                     static_cast<int>(action), static_cast<int>(state));
        });

    // Register event handlers
    event_dispatcher.register_callback(
        jenlib::events::EventType::kBleMessage,
        handle_ble_message_event);
    event_dispatcher.register_callback(
        jenlib::events::EventType::kConnectionStateChange,
        handle_connection_state_event);
    event_dispatcher.register_callback(
        jenlib::events::EventType::kTimeTick,
        handle_time_tick_event);

    ESP_LOGI(TAG, "Sensor initialized successfully");

    // Main loop
    while (true) {
        // Process all event systems
        event_dispatcher.process_events();
        sensor.process_events();
        jenlib::time::Time::process_timers();

        // Process state machine events
        // The state machine handles its own event processing internally

        vTaskDelay(pdMS_TO_TICKS(10));  // 10ms delay
    }
}

//! @section Implementations of forward declared functions
void callback_connection(bool connected) {
    ESP_LOGI(TAG, "BLE connection: %s", connected ? "connected" : "disconnected");

    // Update state machine first - this validates the transition
    sensor_state_machine.handle_connection_change(connected);

    // Then dispatch event for other systems that might need to know
    jenlib::events::Event event(
        jenlib::events::EventType::kConnectionStateChange,
        jenlib::time::Time::now(),
        connected ? 1 : 0);
    event_dispatcher.dispatch_event(event);
}

void callback_start(jenlib::ble::DeviceId sender_id, const jenlib::ble::StartBroadcastMsg &msg) {
    ESP_LOGI(TAG, "Received start broadcast from device: 0x%08x", sender_id.value());

    // First check if this message is intended for this sensor
    if (msg.device_id != kDeviceId) {
        ESP_LOGW(TAG, "Start broadcast not for this device (0x%08x)", msg.device_id.value());
        return;
    }

    // Update state machine - this validates we're in the right state (waiting)
    bool success = sensor_state_machine.handle_start_broadcast(sender_id, msg);
    if (success) {
        ESP_LOGI(TAG, "Starting measurement session");
        // Only start session if state machine allows it
        start_measurement_session(msg);
    } else {
        ESP_LOGW(TAG, "Failed to start measurement session - invalid state");
    }

    // Dispatch BLE message event
    jenlib::events::Event event(
        jenlib::events::EventType::kBleMessage,
        jenlib::time::Time::now(),
        static_cast<std::uint32_t>(jenlib::ble::MessageType::StartBroadcast));
    event_dispatcher.dispatch_event(event);
}

void callback_receipt(jenlib::ble::DeviceId sender_id, const jenlib::ble::ReceiptMsg &msg) {
    ESP_LOGI(TAG, "Received receipt from device: 0x%08x", sender_id.value());

    // Update state machine
    sensor_state_machine.handle_receipt(sender_id, msg);

    // Dispatch BLE message event
    jenlib::events::Event event(
        jenlib::events::EventType::kBleMessage,
        jenlib::time::Time::now(),
        static_cast<std::uint32_t>(jenlib::ble::MessageType::Receipt));
    event_dispatcher.dispatch_event(event);

    // Handle receipt acknowledgment (could purge buffered readings here)
    // The state machine ensures we're in the right state and session
}

void callback_generic(jenlib::ble::DeviceId sender_id, const jenlib::ble::BlePayload &payload) {
    ESP_LOGI(TAG, "Received generic message from device: 0x%08x", sender_id.value());

    // Dispatch generic BLE message event
    jenlib::events::Event event(
        jenlib::events::EventType::kBleMessage,
        jenlib::time::Time::now(),
        static_cast<std::uint32_t>(jenlib::ble::MessageType::Reading));
    event_dispatcher.dispatch_event(event, nullptr);
}

//! @section Implementations of event handler functions
void handle_measurement_timer() {
    // Let the state machine handle the measurement logic
    sensor_state_machine.handle_measurement_timer();
    take_and_broadcast_reading();
}

void handle_time_tick_event(const jenlib::events::Event& event) {
    // Forward time tick events to the state machine
    sensor_state_machine.handle_event(event);
}

void handle_ble_message_event(const jenlib::events::Event& event) {
    // This could be used for logging or additional processing
    // The actual message handling is done in the BLE callbacks
    ESP_LOGD(TAG, "BLE message event processed");
}

void handle_connection_state_event(const jenlib::events::Event& event) {
    bool connected = (event.data != 0);
    if (!connected && sensor_state_machine.is_session_active()) {
        ESP_LOGW(TAG, "Connection lost during active session, stopping measurements");
        // Connection lost, stop measurement session
        sensor_state_machine.handle_session_end();
        stop_measurement_session();
    }
}

//! @section Implementations of helper functions
void start_measurement_session(const jenlib::ble::StartBroadcastMsg& msg) {
    ESP_LOGI(TAG, "Starting measurement session");

    // Stop any existing session
    stop_measurement_session();

    // Configure state machine with session parameters
    sensor_state_machine.set_measurement_interval_ms(1000);  // 1 second interval

    // Schedule first measurement immediately
    take_and_broadcast_reading();

    // Schedule recurring measurements using state machine's timer
    jenlib::time::schedule_repeating_timer(
        sensor_state_machine.get_measurement_interval_ms(),
        handle_measurement_timer);
}

void stop_measurement_session() {
    ESP_LOGI(TAG, "Stopping measurement session");
    // The state machine handles session state management
    // This function can be used for cleanup if needed
}

void take_and_broadcast_reading() {
    if (!sensor_state_machine.is_session_active()) {
        return;
    }

    // Read sensors
    float temperature_c = read_temperature_sensor();
    float humidity_pct = read_humidity_sensor();

    // Create reading message using state machine's session info
    jenlib::ble::ReadingMsg reading_msg{
        .sender_id = kDeviceId,
        .session_id = sensor_state_machine.get_current_session_id(),
        .offset_ms = jenlib::time::Time::now(),  // Simplified for this example
        .temperature_c_centi = jenlib::measurement::temperature_to_centi(temperature_c),
        .humidity_bp = jenlib::measurement::humidity_to_basis_points(humidity_pct)
    };

    // Broadcast the reading
    sensor.broadcast_reading(reading_msg);

    ESP_LOGI(TAG, "Broadcasted reading: temp=%.1f°C, humidity=%.1f%%",
             temperature_c, humidity_pct);
}

//! @section Implementations of mock sensor reading functions
float read_temperature_sensor() {
    // Mock temperature reading - replace with actual sensor code
    // For demo purposes, return a simulated temperature
    static float base_temp = 22.5f;
    static float variation = 0.0f;
    variation += 0.1f;
    if (variation > 2.0f) variation = -2.0f;
    return base_temp + variation;
}

float read_humidity_sensor() {
    // Mock humidity reading - replace with actual sensor code
    // For demo purposes, return a simulated humidity
    static float base_humidity = 45.0f;
    static float variation = 0.0f;
    variation += 0.2f;
    if (variation > 5.0f) variation = -5.0f;
    return base_humidity + variation;
}

//! @section ESP-IDF app_main function
extern "C" void app_main() {
    ESP_LOGI(TAG, "Starting JenLib ESP-IDF sensor example");

    // Create sensor task
    xTaskCreate(sensor_task, "sensor_task", 8192, nullptr, 5, nullptr);

    ESP_LOGI(TAG, "Sensor task created, system running");
}
