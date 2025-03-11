#pragma once

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace della_ac {

class DellaAC : public climate::Climate, public Component {
 public:
  DellaAC() = default;

  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_uart(uart::UARTComponent *uart) { this->uart_ = uart; }

 protected:
  // Climate control functions
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

  // Communication with AC
  void read_ac();
  void send_ac(uint8_t power = 255, float temp_set = 255, uint8_t halbe = 0, 
               uint8_t speed = 255, uint8_t eco = 255, uint8_t turbo = 255, uint8_t mode = 255);

  // Variables from Klima04.txt
  uart::UARTComponent *uart_{nullptr};
  uint8_t rec_a_[80];
  uint8_t trx_a_[8] = {0xBB, 0x00, 0x01, 0x04, 0x02, 0x01, 0x00, 0xBD};
  uint8_t snd_a_[35] = {0xBB, 0x00, 0x01, 0x03, 0x1D, 0x00, 0x00, 0x64, 0x01, 0x59, 0x00, 0x00, 0x00, 
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x18};
  
  uint8_t speeds_[6] = {0, 2, 6, 3, 7, 5};
  uint8_t modes_[5] = {3, 7, 2, 1, 8};
  
  uint8_t power_ = 0, eco_ = 0, turbo_ = 0;
  uint8_t current_mode_ = 4, current_speed_ = 1;
  float current_temp_set_ = 21, room_temp_ = 0, pipe_temp_ = 0, outdoor_temp_ = 0, outdoor_pipe_temp_ = 0;

  unsigned long last_read_time_ = 0;
  unsigned long last_command_time_ = 0;
};

}  // namespace della_ac
}  // namespace esphome 