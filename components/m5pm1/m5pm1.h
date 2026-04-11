#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace m5pm1 {

class M5PM1Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  
  void trigger_update();

  void set_battery_voltage_sensor(sensor::Sensor *s) { battery_voltage_sensor_ = s; }
  void set_on_battery_sensor(binary_sensor::BinarySensor *s) { on_battery_sensor_ = s; }
  void set_on_usb_sensor(binary_sensor::BinarySensor *s) { on_usb_sensor_ = s; }
  
  void set_update_interval(uint32_t interval) { this->update_interval_ = interval; }

  void set_ldo_enabled(bool on);
  void set_dcdc_enabled(bool on);
  void set_5v_enabled(bool on);  // L3A: 5V output enable (standby ON)
  void set_charging_enabled(bool on);
  void set_standby(bool on);    // L3A/L3B control
  void shutdown(bool hold = true);

 protected:
  void read_sensors_();
  uint8_t read_reg_(uint8_t reg);
  void write_reg_(uint8_t reg, uint8_t val);
  uint16_t read_battery_voltage_mv_();

  sensor::Sensor *battery_voltage_sensor_{nullptr};
  binary_sensor::BinarySensor *on_battery_sensor_{nullptr};
  binary_sensor::BinarySensor *on_usb_sensor_{nullptr};

  static const uint8_t REG_PWR_SRC = 0x04;
  static const uint8_t REG_PWR_CFG = 0x06;
  static const uint8_t REG_GPIO_FUNC0 = 0x16;
  static const uint8_t REG_GPIO_MODE0 = 0x10;
  static const uint8_t REG_GPIO_DRV = 0x13;
  static const uint8_t REG_GPIO_OUT = 0x11;
  static const uint8_t REG_SYS_CMD = 0x0C;
  static const uint8_t REG_VBAT_L = 0x22;
  static const uint8_t REG_VBAT_H = 0x23;
};

}  // namespace m5pm1
}  // namespace esphome