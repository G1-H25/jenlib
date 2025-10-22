# State Machine Examples

## Sensor State Machine

```cpp
#include <jenlib/state/SensorStateMachine.h>

jenlib::state::SensorStateMachine sensor_state_machine;

// Handle BLE connection changes
void callback_connection(bool connected) {
    sensor_state_machine.handle_connection_change(connected);
}

// Handle start broadcast messages
void callback_start(jenlib::ble::DeviceId sender_id,
                   const jenlib::ble::StartBroadcastMsg &msg) {
    bool success = sensor_state_machine.handle_start_broadcast(sender_id, msg);
    if (success) {
        start_measurement_session(msg);
    }
}
```

## State Validation

```cpp
void take_reading() {
    if (sensor_state_machine.is_session_active()) {
        // Take and broadcast reading
    }
}
```

## States

- `kDisconnected` - Not connected to broker
- `kWaiting` - Connected, waiting for start command
- `kRunning` - Actively broadcasting measurements
- `kError` - Error state
