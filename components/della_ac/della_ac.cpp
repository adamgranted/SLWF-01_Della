#include "della_ac.h"
#include "esphome/core/log.h"

namespace esphome {
namespace della_ac {

static const char *TAG = "della_ac";

void DellaAC::setup() {
  // Initial read after component setup
  this->read_ac();
  
  // Set up initial state
  this->mode = climate::CLIMATE_MODE_OFF;
  this->target_temperature = this->current_temp_set_;
  this->current_temperature = this->room_temp_;
  
  // Publish initial state
  this->publish_state();
}

void DellaAC::dump_config() {
  ESP_LOGCONFIG(TAG, "Della AC:");
}

void DellaAC::loop() {
  const uint32_t now = millis();
  
  // Read from AC every 30 seconds
  if (now - this->last_read_time_ >= 30000) {
    this->read_ac();
    this->last_read_time_ = now;
  }
}

climate::ClimateTraits DellaAC::traits() {
  auto traits = climate::ClimateTraits();
  
  // Basic climate capabilities
  traits.set_supports_current_temperature(true);
  
  // Operating modes
  traits.set_supported_modes({
    climate::CLIMATE_MODE_OFF,
    climate::CLIMATE_MODE_AUTO,
    climate::CLIMATE_MODE_COOL,
    climate::CLIMATE_MODE_HEAT,
    climate::CLIMATE_MODE_FAN_ONLY,
    climate::CLIMATE_MODE_DRY
  });
  
  // Fan modes
  traits.set_supported_fan_modes({
    climate::CLIMATE_FAN_AUTO,
    climate::CLIMATE_FAN_LOW,
    climate::CLIMATE_FAN_MEDIUM,
    climate::CLIMATE_FAN_HIGH,
    climate::CLIMATE_FAN_MIDDLE,
  });
  
  // Presets (ECO/TURBO in the original code)
  traits.set_supported_presets({
    climate::CLIMATE_PRESET_ECO,
    climate::CLIMATE_PRESET_BOOST,
  });
  
  // Temperature range
  traits.set_visual_min_temperature(16);
  traits.set_visual_max_temperature(30);
  traits.set_visual_temperature_step(0.5);
  
  return traits;
}

void DellaAC::control(const climate::ClimateCall &call) {
  uint8_t send_power = 255;
  float send_temp = 255;
  uint8_t halbe = 0;
  uint8_t send_speed = 255;
  uint8_t send_eco = 255;
  uint8_t send_turbo = 255;
  uint8_t send_mode = 255;
  
  // Handle mode changes
  if (call.get_mode().has_value()) {
    climate::ClimateMode mode = *call.get_mode();
    
    // Handle power on/off
    if (mode == climate::CLIMATE_MODE_OFF) {
      send_power = 0;
    } else if (this->mode == climate::CLIMATE_MODE_OFF) {
      send_power = 1;
    }
    
    // Handle mode changes
    if (mode == climate::CLIMATE_MODE_AUTO) {
      send_mode = 5;  // Auto mode
    } else if (mode == climate::CLIMATE_MODE_COOL) {
      send_mode = 1;  // Cooling mode
    } else if (mode == climate::CLIMATE_MODE_HEAT) {
      send_mode = 4;  // Heating mode
    } else if (mode == climate::CLIMATE_MODE_FAN_ONLY) {
      send_mode = 2;  // Ventilation mode
    } else if (mode == climate::CLIMATE_MODE_DRY) {
      send_mode = 3;  // Drying mode
    }
  }
  
  // Handle target temperature changes
  if (call.get_target_temperature().has_value()) {
    float temp = *call.get_target_temperature();
    send_temp = temp;
    
    // Handle half-degree settings
    float temp_int = static_cast<int>(temp);
    if (temp - temp_int >= 0.49) {
      halbe = 1;
    } else {
      halbe = 0;
    }
  }
  
  // Handle fan mode changes
  if (call.get_fan_mode().has_value()) {
    climate::ClimateFanMode fan_mode = *call.get_fan_mode();
    
    if (fan_mode == climate::CLIMATE_FAN_AUTO) {
      send_speed = 1;  // Auto fan speed
    } else if (fan_mode == climate::CLIMATE_FAN_LOW) {
      send_speed = 2;  // Low speed
    } else if (fan_mode == climate::CLIMATE_FAN_MEDIUM || fan_mode == climate::CLIMATE_FAN_MIDDLE) {
      send_speed = 3;  // Medium speed
    } else if (fan_mode == climate::CLIMATE_FAN_HIGH) {
      send_speed = 5;  // High speed
    }
  }
  
  // Handle preset changes
  if (call.get_preset().has_value()) {
    climate::ClimatePreset preset = *call.get_preset();
    
    if (preset == climate::CLIMATE_PRESET_ECO) {
      send_eco = 1;
      send_turbo = 0;
    } else if (preset == climate::CLIMATE_PRESET_BOOST) {
      send_eco = 0;
      send_turbo = 1;
    } else {
      send_eco = 0;
      send_turbo = 0;
    }
  }
  
  // Send the command to the AC
  this->send_ac(send_power, send_temp, halbe, send_speed, send_eco, send_turbo, send_mode);
  
  // The read_ac function will be called by send_ac and will update our state
}

void DellaAC::send_ac(uint8_t power, float temp_set, uint8_t halbe, uint8_t speed, uint8_t eco, uint8_t turbo, uint8_t mode) {
  // Power control
  if (power < 2) {
    this->snd_a_[7] = (this->snd_a_[7] & 0xFB) | (4 * power);
  } else {
    this->snd_a_[7] = (this->snd_a_[7] & 0xFB) | (4 * this->power_);
  }
  
  // Mode control
  if (mode < 6) {
    this->snd_a_[8] = (this->snd_a_[8] & 0xF0) | (this->modes_[mode - 1]);
  } else {
    this->snd_a_[8] = (this->snd_a_[8] & 0xF0) | (this->modes_[this->current_mode_ - 1]);
  }
  
  // Turbo mode
  if (turbo < 2) {
    this->snd_a_[8] = (this->snd_a_[8] & 0xBF) | (0x40 * turbo);
  } else {
    this->snd_a_[8] = (this->snd_a_[8] & 0xBF) | (0x40 * this->turbo_);
  }
  
  // Eco mode
  if (eco < 2) {
    this->snd_a_[7] = (this->snd_a_[7] & 0x7F) | (0x80 * eco);
  } else {
    this->snd_a_[7] = (this->snd_a_[7] & 0x7F) | (0x80 * this->eco_);
  }
  
  // Temperature setting
  if (temp_set < 32 && temp_set > 15) {
    this->snd_a_[9] = 0x6F - temp_set;
    if (halbe == 1) {
      this->snd_a_[11] = this->snd_a_[11] | 0x02;
    } else {
      this->snd_a_[11] = this->snd_a_[11] & 0xFD;
    }
  } else {
    this->snd_a_[9] = 0x6F - this->current_temp_set_;
  }
  
  // Fan speed
  if (speed < 6) {
    this->snd_a_[10] = this->speeds_[speed];
  } else {
    this->snd_a_[10] = this->speeds_[this->current_speed_];
  }
  
  // Calculate checksum
  uint8_t csum = 0;
  for (uint8_t cnt = 0; cnt <= 33; cnt++) {
    csum = csum ^ this->snd_a_[cnt];
  }
  this->snd_a_[34] = csum;
  
  // Send the command
  this->uart_->write_array(this->snd_a_, 35);
  
  // Wait for response
  delay(700);
  
  // Read updated state
  this->read_ac();
  
  // Update last command time
  this->last_command_time_ = millis();
}

void DellaAC::read_ac() {
  bool got_frame = false;
  uint8_t index = 0;
  
  // Send request to AC
  this->uart_->write_array(this->trx_a_, 8);
  
  // Wait for response with timeout
  unsigned long start_time = millis();
  while ((millis() - start_time) < 500) {
    if (this->uart_->available()) {
      uint8_t data;
      if (this->uart_->read_byte(&data)) {
        // Check for frame start
        if (index == 0 && data == 0xBB) {
          this->rec_a_[0] = data;
          index = 1;
          continue;
        }
        
        // Continue reading frame
        if (index > 0) {
          this->rec_a_[index] = data;
          index++;
          
          // Prevent buffer overflow
          if (index >= 80) {
            break;
          }
        }
      }
    }
    yield();  // Allow ESP8266 to handle background tasks
  }
  
  // Process received data
  if (this->rec_a_[0] == 0xBB && this->rec_a_[1] == 1) {
    // Update our internal state based on response
    
    // Temperature settings
    this->current_temp_set_ = (this->rec_a_[8] & 0x0F) + 16 + ((this->rec_a_[9] & 0x02) * 0.25);
    float halbe = ((this->rec_a_[9] & 0x02) * 0.5);
    
    // Power/Eco/Turbo status
    this->power_ = (this->rec_a_[7] & 0x10) > 0 ? 1 : 0;
    this->eco_ = (this->rec_a_[7] & 0x40) > 0 ? 1 : 0;
    this->turbo_ = (this->rec_a_[7] & 0x80) > 0 ? 1 : 0;
    
    // Mode
    this->current_mode_ = (this->rec_a_[7] & 0x0F);
    
    // Fan speed
    uint8_t zahl = (this->rec_a_[8] & 0xF0);
    if (zahl == 0x80) this->current_speed_ = 0;
    else if (zahl == 0x90) this->current_speed_ = 1;
    else if (zahl == 0xC0) this->current_speed_ = 2;
    else if (zahl == 0xA0) this->current_speed_ = 3;
    else if (zahl == 0xD0) this->current_speed_ = 4;
    else if (zahl == 0xB0) this->current_speed_ = 5;
    
    // Various temperature readings
    this->room_temp_ = (this->rec_a_[17] - 65) / 2.0f;
    this->pipe_temp_ = (this->rec_a_[30] - 65) / 2.0f;
    this->outdoor_pipe_temp_ = this->rec_a_[37] - 16;
    this->outdoor_temp_ = this->rec_a_[36] - 16;
    
    // Update ESPHome climate component state
    
    // Power state
    if (this->power_ == 0) {
      this->mode = climate::CLIMATE_MODE_OFF;
    } else {
      // Map mode
      switch (this->current_mode_) {
        case 1:
          this->mode = climate::CLIMATE_MODE_COOL;
          break;
        case 2:
          this->mode = climate::CLIMATE_MODE_FAN_ONLY;
          break;
        case 3:
          this->mode = climate::CLIMATE_MODE_DRY;
          break;
        case 4:
          this->mode = climate::CLIMATE_MODE_HEAT;
          break;
        case 5:
          this->mode = climate::CLIMATE_MODE_AUTO;
          break;
        default:
          this->mode = climate::CLIMATE_MODE_AUTO;
          break;
      }
    }
    
    // Fan speed
    switch (this->current_speed_) {
      case 0:
        this->fan_mode = climate::CLIMATE_FAN_AUTO;
        break;
      case 1:
        this->fan_mode = climate::CLIMATE_FAN_LOW;
        break;
      case 2:
        this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
        break;
      case 3:
        this->fan_mode = climate::CLIMATE_FAN_HIGH;
        break;
      case 4:
      case 5:
        this->fan_mode = climate::CLIMATE_FAN_HIGH;
        break;
      default:
        this->fan_mode = climate::CLIMATE_FAN_AUTO;
        break;
    }
    
    // Preset (ECO/TURBO)
    if (this->eco_ == 1) {
      this->preset = climate::CLIMATE_PRESET_ECO;
    } else if (this->turbo_ == 1) {
      this->preset = climate::CLIMATE_PRESET_BOOST;
    } else {
      this->preset = climate::CLIMATE_PRESET_NONE;
    }
    
    // Temperature
    this->target_temperature = this->current_temp_set_;
    this->current_temperature = this->room_temp_;
    
    // Publish the updated state
    this->publish_state();
  }
}

}  // namespace della_ac
}  // namespace esphome 