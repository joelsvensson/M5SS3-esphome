#include "m5pm1.h"
#include "esphome/core/log.h"

namespace esphome {
namespace m5pm1 {

static const char *const TAG = "m5pm1";

void M5PM1Component::setup() {
  ESP_LOGCONFIG(TAG, "M5PM1 @ 0x%02X", this->address_);
  // read_reg_ handles wake-up automatically now
}

void M5PM1Component::update() {
  read_sensors_();
}

void M5PM1Component::trigger_update() {
  read_sensors_();
}

void M5PM1Component::read_sensors_() {
  if (battery_voltage_sensor_) {
    uint16_t voltage_mv = read_battery_voltage_mv_();
    float voltage = voltage_mv / 1000.0f;
    battery_voltage_sensor_->publish_state(voltage);
  }

  if (on_battery_sensor_ || on_usb_sensor_) {
    uint8_t src = read_reg_(REG_PWR_SRC);
    bool on_bat = (src == 2);
    bool on_usb = (src == 0) || (src == 1);
    if (on_battery_sensor_) on_battery_sensor_->publish_state(on_bat);
    if (on_usb_sensor_) on_usb_sensor_->publish_state(on_usb);
  }
}

void M5PM1Component::dump_config() {
  ESP_LOGCONFIG(TAG, "M5PM1:");
  LOG_I2C_DEVICE(this);
  ESP_LOGD(TAG, " Update interval: %u ms", this->update_interval_);
  uint8_t pwr_src = read_reg_(REG_PWR_SRC);
  uint8_t pwr_cfg = read_reg_(REG_PWR_CFG);
  ESP_LOGD(TAG, "PWR_SRC=0x%02X PWR_CFG=0x%02X", pwr_src, pwr_cfg);
}

uint8_t M5PM1Component::read_reg_(uint8_t reg) {
  uint8_t val = 0;
  
  // Wake-up: read REG 0x06 first to wake PM1 from sleep
  //uint8_t wake = 0;
  //this->read_register(0x06, &wake, 1);
  
  // Now do intended read 
  if (this->read_register(reg, &val, 1) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "I2C read failed: 0x%02X", reg);
  }
  return val;
}

void M5PM1Component::write_reg_(uint8_t reg, uint8_t val) {
  // Wake-up read first
  uint8_t wake = 0;
  this->read_register(0x06, &wake, 1);
  
  // Now do intended write
  if (this->write_register(reg, &val, 1) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "I2C write failed: 0x%02X", reg);
  }
}

uint16_t M5PM1Component::read_battery_voltage_mv_() {
  uint8_t buf[2] = {0};
  this->read_register(REG_VBAT_L, buf, 2);
  return (buf[1] << 8) | buf[0];
}

void M5PM1Component::set_ldo_enabled(bool on) {
  uint8_t cfg = read_reg_(REG_PWR_CFG);
  if (on) cfg |= (1 << 2);
  else cfg &= ~(1 << 2);
  write_reg_(REG_PWR_CFG, cfg);
}
void M5PM1Component::set_dcdc_enabled(bool on) {
  uint8_t cfg = read_reg_(REG_PWR_CFG);
  if (on) cfg |= (1 << 1);
  else cfg &= ~(1 << 1);
  write_reg_(REG_PWR_CFG, cfg);
}
void M5PM1Component::set_5v_enabled(bool on) {
  uint8_t cfg = read_reg_(REG_PWR_CFG);
  if (on) cfg |= (1 << 3);
  else cfg &= ~(1 << 3);
  write_reg_(REG_PWR_CFG, cfg);
}
void M5PM1Component::set_charging_enabled(bool on) {
  uint8_t cfg = read_reg_(REG_PWR_CFG);
  if (on) cfg |= (1 << 0);
  else cfg &= ~(1 << 0);
  write_reg_(REG_PWR_CFG, cfg);
}

void M5PM1Component::set_standby(bool on) {
  // L3A = standby (low power), L3B = high power (all peripherals)
  // Control via M5PM1 GPIO2 (PYG2 pin)
  // Set GPIO2 function: REG 0x16 bit 2 = 0 (GPIO function)
  uint8_t func = read_reg_(REG_GPIO_FUNC0);
  func &= ~(1 << 2);  // Clear bit 2 for GPIO function
  write_reg_(REG_GPIO_FUNC0, func);
  
  // Set GPIO2 mode: REG 0x10 bit 2 = 0 (output)
  uint8_t mode = read_reg_(REG_GPIO_MODE0);
  mode &= ~(1 << 2);  // Clear bit 2 for output
  write_reg_(REG_GPIO_MODE0, mode);
  
  // Set output level via REG 0x11 bit 2
  // false = L3B enabled (high power), true = L3A enabled (standby/low power)
  uint8_t out = read_reg_(REG_GPIO_OUT);
  if (on) {
    out |= (1 << 2);   // L3A = standby ON
  } else {
    out &= ~(1 << 2); // L3B = high power ON
  }
  write_reg_(REG_GPIO_OUT, out);
  
  ESP_LOGD(TAG, "Standby: %s (L3%s)", on ? "ON" : "OFF", on ? "A" : "B");
}

void M5PM1Component::shutdown(bool hold) {
  (void)hold;
  write_reg_(REG_SYS_CMD, (0xA << 4) | 0x01);
}

}  // namespace m5pm1
}  // namespace esphome

