#include <jenlib/gpio/GPIO.h>

//! @namespace anonymous namespace
namespace {
    jenlib::gpio::GpioDriver* g_driver = nullptr;
    std::uint8_t g_ar_bits = 10;
    std::uint8_t g_aw_bits = 8;
}

//! @namespace GPIO
//! @brief Public wrapper API for GPIO operations.
//! @note If driver is not set, all operations return as noop.
namespace GPIO {

Pin::Pin() noexcept : raw_pin_(0) {}
Pin::Pin(PinIndex pin_index) noexcept : raw_pin_(pin_index) {}



void Pin::pinMode(PinMode mode) const noexcept {
    if (!g_driver) return;
    g_driver->set_pin_mode(raw_pin_, static_cast<jenlib::gpio::PinMode>(mode));
}

void Pin::digitalWrite(DigitalValue value) const noexcept {
    if (!g_driver) return;
    g_driver->digital_write(raw_pin_, static_cast<jenlib::gpio::DigitalValue>(value));
}

void Pin::analogWrite(std::uint16_t value) const noexcept {
    if (!g_driver) return;
    g_driver->analog_write(raw_pin_, value);
}

DigitalValue Pin::digitalRead() const noexcept {
    if (!g_driver) return DigitalValue::LOW;
    return static_cast<DigitalValue>(static_cast<std::uint8_t>(g_driver->digital_read(raw_pin_)));
}

std::uint16_t Pin::analogRead() const noexcept {
    if (!g_driver) return 0;
    return g_driver->analog_read(raw_pin_);
}

PinMap::PinMap() {}
PinMap::~PinMap() {}
Pin PinMap::operator[](PinIndex index) const noexcept { return Pin(index); }

void setAnalogReadResolution(std::uint8_t bits) noexcept {
    g_ar_bits = bits;
    if (g_driver) g_driver->set_analog_read_resolution(bits);
}

void setAnalogWriteResolution(std::uint8_t bits) noexcept {
    g_aw_bits = bits;
    if (g_driver) g_driver->set_analog_write_resolution(bits);
}

std::uint8_t getAnalogReadResolution() noexcept {
    if (g_driver) return g_driver->get_analog_read_resolution();
    return g_ar_bits;
}

std::uint8_t getAnalogWriteResolution() noexcept {
    if (g_driver) return g_driver->get_analog_write_resolution();
    return g_aw_bits;
}

void setDriver(jenlib::gpio::GpioDriver* driver) noexcept {
    g_driver = driver;
}

jenlib::gpio::GpioDriver* getDriver() noexcept {
    return g_driver;
}

} // namespace GPIO



