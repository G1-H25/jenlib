# GPIO Examples

## Basic Pin Usage

```cpp
#include <jenlib/gpio/GPIO.h>

// Create pin handles
GPIO::Pin led_pin(13);
GPIO::Pin sensor_pin(A0);

// Configure pins
led_pin.pinMode(GPIO::PinMode::OUTPUT);
sensor_pin.pinMode(GPIO::PinMode::INPUT_PULLUP);

// Use pins
led_pin.digitalWrite(GPIO::DigitalValue::HIGH);
auto value = sensor_pin.digitalRead();
```

## TMP36 Temperature Sensor

```cpp
float read_temperature() {
    GPIO::Pin temp_pin(A0);
    temp_pin.pinMode(GPIO::PinMode::INPUT);

    std::uint16_t analog_value = temp_pin.analogRead();
    float voltage = (analog_value * 5.0f) / 1023.0f;
    float temperature_c = (voltage - 0.5f) * 100.0f;

    return temperature_c;
}
```

## Pin Map Usage

```cpp
GPIO::PinMap pins;
pins[13].pinMode(GPIO::PinMode::OUTPUT);
pins[13].digitalWrite(GPIO::DigitalValue::HIGH);
```
