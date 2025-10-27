# JenLib Integration Guide

This guide explains how to integrate JenLib into ESP-IDF and Arduino projects.

## ESP-IDF Integration

### 1. Component Structure

JenLib is designed as an ESP-IDF component. To use it in your ESP-IDF project:

1. Copy the `components/jenlib` directory to your ESP-IDF project's `components` folder
2. The component will automatically be detected and built

### 2. Required ESP-IDF Components

The JenLib ESP-IDF component requires these ESP-IDF components:
- `driver` - For GPIO operations
- `esp_timer` - For timing functions
- `bt` - For Bluetooth functionality
- `nvs_flash` - For non-volatile storage
- `esp_wifi` - For WiFi (if needed)
- `esp_netif` - For network interface
- `esp_event` - For event handling
- `esp_common` - For common ESP functions

### 3. Usage Example

```cpp
#include <jenlib/gpio/drivers/EspIdfGpioDriver.h>
#include <jenlib/ble/drivers/EspIdfBleDriver.h>
#include <jenlib/time/drivers/EspIdfTimeDriver.h>

// Initialize drivers
jenlib::gpio::EspIdfGpioDriver gpio_driver;
jenlib::ble::EspIdfBleDriver ble_driver("MyDevice", device_id);
jenlib::time::EspIdfTimeDriver time_driver;

// Initialize time service
jenlib::time::Time::initialize();

// Use GPIO
auto led_pin = jenlib::gpio::makeTypedPin<jenlib::gpio::Digital>(GPIO_NUM_2);
led_pin.pinMode(jenlib::gpio::PinMode::kOutput);
led_pin.digitalWrite(jenlib::gpio::DigitalValue::HIGH);
```

### 4. Building

```bash
idf.py build
idf.py flash monitor
```

## Arduino Integration

### 1. Library Installation

#### Option A: Arduino Library Manager
1. Open Arduino IDE
2. Go to Tools → Manage Libraries
3. Search for "jenlib"
4. Click Install

#### Option B: Manual Installation
1. Download the library
2. Extract to your Arduino `libraries` folder
3. Restart Arduino IDE

#### Option C: PlatformIO
Add to your `platformio.ini`:
```ini
lib_deps = 
    ArduinoBLE
    https://github.com/G1-H25/jenlib.git
```

### 2. Required Arduino Libraries

- **ArduinoBLE** (>=1.3.0) - For BLE functionality
- **Arduino** - Core Arduino framework

### 3. Supported Platforms

- ESP32 (espressif32)
- ESP8266 (espressif8266) 
- Arduino Nano 33 BLE (nordicnrf52)
- Teensy 4.1 (teensy)
- STM32 (ststm32)
- Atmel SAM (atmelsam)

### 4. Usage Example

```cpp
#include <jenlib/gpio/drivers/ArduinoGpioDriver.h>
#include <jenlib/ble/drivers/ArduinoBleDriver.h>
#include <jenlib/time/drivers/ArduinoTimeDriver.h>

// Initialize drivers
jenlib::gpio::ArduinoGpioDriver gpio_driver;
jenlib::ble::ArduinoBleDriver ble_driver("MyDevice", device_id);
jenlib::time::ArduinoTimeDriver time_driver;

void setup() {
    // Initialize time service
    jenlib::time::Time::initialize();
    
    // Use GPIO
    auto led_pin = jenlib::gpio::makeTypedPin<jenlib::gpio::Digital>(LED_BUILTIN);
    led_pin.pinMode(jenlib::gpio::PinMode::kOutput);
    led_pin.digitalWrite(jenlib::gpio::DigitalValue::HIGH);
}

void loop() {
    jenlib::time::Time::process_timers();
}
```

## Platform-Specific Features

### ESP-IDF Specific Features
- Direct ESP-IDF GPIO API integration
- ESP-IDF timer system integration
- Native ESP-IDF BLE stack support
- OneWire bus implementation using ESP-IDF GPIO

### Arduino Specific Features
- Arduino GPIO API integration
- Arduino millis() timing system
- ArduinoBLE library integration
- Arduino OneWire library compatibility

## Build Configuration

### CMake Detection
The library automatically detects the build environment:
- Arduino toolchain: `CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_SYSTEM_NAME STREQUAL "Generic"`
- ESP-IDF: Uses ESP-IDF component system
- Native: Uses native drivers for desktop/container environments

### Conditional Compilation
The library uses `#ifdef ARDUINO` guards to conditionally compile platform-specific code:
- Arduino code is only compiled when `ARDUINO` is defined
- ESP-IDF code is only compiled in ESP-IDF environment
- Native code is compiled for desktop/container environments

## Examples

### ESP-IDF Examples
- `examples/esp_idf/ble_sensor/` - Complete BLE sensor implementation

### Arduino Examples
- `examples/arduino/ble_sensor/` - BLE sensor using ArduinoBLE
- `examples/arduino/simple_gpio/` - Basic GPIO operations
- `examples/arduino/temperature/` - Temperature sensor example

## Troubleshooting

### Common Issues

1. **BLE not working on ESP32**
   - Ensure Bluetooth is enabled in ESP-IDF menuconfig
   - Check that the correct BLE driver is being used

2. **GPIO not working**
   - Verify pin numbers are correct for your platform
   - Check that pins are not already in use by other peripherals

3. **Timing issues**
   - Ensure `jenlib::time::Time::process_timers()` is called regularly
   - Check that the time driver is properly initialized

### Debugging
Enable debug output by setting log levels:
- ESP-IDF: `idf.py menuconfig` → Component config → Log output
- Arduino: Use `Serial.println()` for debugging output
