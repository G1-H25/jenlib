#include <jenlib/gpio/GPIO.h>
#include <jenlib/gpio/drivers/ArduinoGpioDriver.h>

gpio::ArduinoGpioDriver driver;

// TMP36 connected to analog pin A0
const uint8_t SENSOR_PIN = A0;

void setup() {
    Serial.begin(115200);
    GPIO::setDriver(&driver);
    // If your board supports resolution API, uncomment to set 10-bit or 12-bit
    // GPIO::setAnalogReadResolution(10);
}

// Example conversion function kept in example (not the library), to show indirection
static float tmp36_celsius_from_code_example(std::uint16_t code, std::uint8_t bits, float vref_volts) {
    if (bits == 0 || vref_volts <= 0.0) return 0.0;
    const float max_code = static_cast<float>((1u << bits) - 1u);
    const float volts = (static_cast<float>(code) / max_code) * vref_volts;
    return (volts - 0.5) * 100.0;
}

void loop() {
    std::uint16_t code = GPIO::Pin(SENSOR_PIN).analogRead();
    const float vref = 3.3f; // change to 5.0f if your board runs at 5V
    const uint8_t bits = GPIO::getAnalogReadResolution();
    float tempC = tmp36_celsius_from_code_example(code, bits, vref);

    Serial.print("TMP36: ");
    Serial.print(code);
    Serial.print(" (raw), ");
    Serial.print(tempC, 2);
    Serial.println(" C");

    delay(1000);
}



