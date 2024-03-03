#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace demo {

static const char *const TAG = "demo22";

class DemoSensor : public sensor::Sensor, public PollingComponent {
 public:
  void update() override {
    //float val = random_float();
    float val = somevar_;
    somevar_ = somevar_ + 0.01;
    bool increasing = this->get_state_class() == sensor::STATE_CLASS_TOTAL_INCREASING;
    if (increasing) {
      float base = std::isnan(this->state) ? 0.0f : this->state;
      this->publish_state(base + val * 10);
    } else {
      if (val < 0.1)
        this->publish_state(NAN);
      else
        this->publish_state(val * 100);
    }
  }
  void dump_config() override {
      LOG_SENSOR(TAG, "Demo Sensor", this);
      ESP_LOGCONFIG(TAG, " Preved!!");
  }

  void set_somevar(float value) {
    this->somevar_ = value;
  }
  float get_somevar() {
    float temp = 2.0 * (this->somevar_);
    return temp; 
  }

 protected:
  float somevar_{0.0};
};

}  // namespace demo
}  // namespace esphome
