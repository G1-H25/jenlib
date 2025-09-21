//! @file examples/arduino/ble_sensor/main.cpp
//! @brief Arduino code for BLE sensor
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)


#include <Arduino.h>
#include <jenlib/time/Time.h> //!< Should pull in all needed headers for time abstraction
#include <jenlib/measurement/Measurement.h> //!< Should pull in all needed headers for measurement abstraction
#include <jenlib/gpio/GPIO.h> //!< Should pull in all needed headers for GPIO abstraction
#include <jenlib/ble/Ble.h> //!< Should pull in all needed headers for BLE operation


constexpr DeviceId kDeviceId = DeviceId(0x12345678); //! Some DeviceID


enum class ActivityState {
    kDisconnected, //!< Connection not yet established
    kConnected, //!< Connection established
    kPassiveWaiting, //!< Broker detected, waiting for callback to start broadcasting
    kBroadcasting, //!< Broadcasting
    kNumStates //!< Number of states
};

ActivityState g_state = ActivityState::kDisconnected; //!< Current state

void setup() {
    jenlib::GPIO::setDriver(new gpio::ArduinoGpioDriver()); //!< Set the GPIO driver to ArduinoGpioDriver for actual Arduino implementation
    jenlib::ble::BLE::set_driver(new jenlib::ble::ArduinoBleDriver("Sensor", jenlib::ble::DeviceId(static_cast<std::uint32_t>(kDeviceId)))); //!< Set the BLE driver to ArduinoBleDriver for actual Arduino implementation

    jenlib::ble::BLE::begin();

    while (!jenlib::ble::BLE::is_connected()) {
        jenlib::time::delay(1000); //!< Wait for connection
    }

    g_state = ActivityState::kConnected; //!< Set the state to connected

}

void loop() {
    switch (g_state) {
        case ActivityState::kDisconnected:
            break; //! No beacon was detected, so we are disconnected
        case ActivityState::kConnected:
            break; //! We are connected to a broker, so we can subscribe to GATT service for our own service
        case ActivityState::kPassiveWaiting:
            break; //! We are waiting for a broker to detect our service and callback to start broadcasting
        case ActivityState::kBroadcasting:
        
        auto time_since_last_broadcast = jenlib::time::now() - last_broadcast_time;
            while (time_since_last_broadcast > kBroadcastInterval) {
                // Placeholder: adapt to new broadcast API if needed
                last_broadcast_time = jenlib::time::now();
            }
            break; //! We are broadcasting our service
        case ActivityState::kNumStates:
            break; //! Should never happen

    }

    //! idle waiting for next loop
    jenlib::time::delay(1000);
}


