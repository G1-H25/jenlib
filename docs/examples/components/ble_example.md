# BLE Examples

## Sensor Setup

```cpp
#include <jenlib/ble/Roles.h>

constexpr jenlib::ble::DeviceId kDeviceId = jenlib::ble::DeviceId(0x12345678);
static jenlib::ble::Sensor sensor(kDeviceId);

// Configure callbacks
sensor.configure_callbacks(jenlib::ble::BleCallbacks{
    .on_connection = callback_connection,
    .on_start = callback_start,
    .on_receipt = callback_receipt,
    .on_generic = callback_generic,
});

// Start BLE
sensor.begin();
```

## Broker Setup

```cpp
static jenlib::ble::Broker broker;

// Start measurement session
jenlib::ble::StartBroadcastMsg start_msg{
    .device_id = target_sensor_id,
    .session_id = jenlib::ble::SessionId::generate()
};
broker.start_measurement_session(start_msg);
```

## Broadcasting Readings

```cpp
jenlib::ble::ReadingMsg reading_msg{
    .sender_id = kDeviceId,
    .session_id = session_id,
    .offset_ms = 1000,
    .temperature_c_centi = 2312,  // 23.12°C
    .humidity_bp = 4500           // 45.00%
};
sensor.broadcast_reading(reading_msg);
```
