#include "lm75.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lm75 {

static const uint8_t LM75_REGISTER_TEMP = 0x00;   // Temperature register
static const uint8_t LM75_REGISTER_CONFIG = 0x01; // Configuration register
static const uint8_t LM75_REGISTER_THYST = 0x02;  // Hysteresis register
static const uint8_t LM75_REGISTER_TOS = 0x03;    // Over-temperature shutdown register

static const char *const TAG = "lm75";

static const float LM75_CONVERSION_FACTOR = 0.125;
static const uint8_t LM75_REGISTER_DATA_SHIFT = 5;

void LM75Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up LM75...");
}

void LM75Component::dump_config() {
  ESP_LOGCONFIG(TAG, "LM75:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with LM75 failed!");
  }
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Temperature", this);
}

void LM75Component::update() {
  uint16_t raw_temp_reg;
  
  // Write pointer to temperature register with error logging
  if (this->write(&LM75_REGISTER_TEMP, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "I2C ERROR: Failed to write to LM75 at address 0x%02X", this->address);
    this->status_set_warning();
    return;
  }
  
  // Read 2 raw bytes from temperature register with error logging
  if (this->read(reinterpret_cast<uint8_t *>(&raw_temp_reg), 2) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "I2C ERROR: Failed to read from LM75 at address 0x%02X", this->address);
    this->status_set_warning();
    return;
  }
  
  // Convert bytes to host order and handle signed 16-bit value (two's complement) for negative temperatures
  int16_t raw_temperature = static_cast<int16_t>(i2c::i2ctohs(raw_temp_reg));
  raw_temperature = raw_temperature >> LM75_REGISTER_DATA_SHIFT;
  float temperature = raw_temperature * LM75_CONVERSION_FACTOR;

  // Log temperature using integer formatting for ESP8266 compatibility
  int temp_int = (int)(temperature * 10);
  ESP_LOGD(TAG, "Got Temperature=%d.%d°C from address 0x%02X", temp_int / 10, abs(temp_int % 10), this->address);

  this->publish_state(temperature);
  this->status_clear_warning();
}

float LM75Component::get_setup_priority() const { 
  return setup_priority::DATA; 
}

} // namespace lm75
} // namespace esphome
