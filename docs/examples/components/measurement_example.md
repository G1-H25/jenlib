# Measurement Examples

## Basic Conversion

```cpp
#include <jenlib/measurement/Measurement.h>

// Read sensor values
float temperature_c = 23.12f;
float humidity_pct = 45.0f;

// Convert for BLE transmission
auto temp_centi = jenlib::measurement::temperature_to_centi(temperature_c);
auto hum_bp = jenlib::measurement::humidity_to_basis_points(humidity_pct);
```

## Creating Measurement Messages

```cpp
jenlib::ble::ReadingMsg reading_msg{
    .sender_id = kDeviceId,
    .session_id = session_id,
    .offset_ms = jenlib::time::Time::now(),
    .temperature_c_centi = temp_centi,  // 2312 (23.12°C)
    .humidity_bp = hum_bp              // 4500 (45.00%)
};
```

## Unit Conversions

- Temperature: Celsius to centi-degrees (multiply by 100)
- Humidity: Percentage to basis points (multiply by 100)
- Time: Platform-specific to milliseconds
