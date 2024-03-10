#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/sensor/sensor.h"
#include <string>

namespace esphome {
namespace demo22 {

static const char *const TAG = "demo22";

class DemoSensor : public sensor::Sensor, public PollingComponent {
  static const int STRING_ARRAY_SAVER_DB_SIZE = 100;

  std::string *phone_prt[STRING_ARRAY_SAVER_DB_SIZE]{};
  std::string *name_prt[STRING_ARRAY_SAVER_DB_SIZE]{};

 public:
  void update() override {
    // float val = random_float();
    float val = somevar_;
    // somevar_ = somevar_ + 0.01;
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
    LOG_SENSOR("", "Demo Sensor", this);
    ESP_LOGCONFIG(TAG, " Preved!!");
  }

  void set_somevar(float value) { this->somevar_ = value; }
  float get_somevar() {
    float temp = 2.0 * (this->somevar_);
    return temp;
  }
  void array_init(std::string *(&p_phone_prt)[STRING_ARRAY_SAVER_DB_SIZE],
                  std::string *(&p_name_prt)[STRING_ARRAY_SAVER_DB_SIZE]) {
    for (size_t i = 0; i < STRING_ARRAY_SAVER_DB_SIZE; i++) {
      phone_prt[i] = p_phone_prt[i];
      name_prt[i] = p_name_prt[i];
    }
  }

  void string_array_saver(std::string *(&p_phone_prt)[STRING_ARRAY_SAVER_DB_SIZE],
                          std::string *(&p_name_prt)[STRING_ARRAY_SAVER_DB_SIZE]) {
    this->array_init(p_phone_prt, p_name_prt);
  }

  std::string show_rec(int num, bool show_undefined) {
    std::string output{"record №"};
    output += std::to_string(num);
    if (phone_prt[num] == nullptr) {
      if (show_undefined) {
        return output + " is undefined";
      } else {
        return "";
      }
    }
    output += " - phone:";
    output += phone_prt[num]->c_str();
    output += ", name:";
    if (name_prt[num] != nullptr) {
      output += name_prt[num]->c_str();
    } else {
      output += "-нет-";
    }
    return output;
  }

  std::string show_rec(int num) { return show_rec(num, false); };

  void print_base(std::string &output, bool show_undefined) {
    output = "Customers\n";
    std::string record;
    for (size_t i = 0; i < STRING_ARRAY_SAVER_DB_SIZE; i++) {
      record = this->show_rec(i, show_undefined);
      if (!show_undefined && record.empty())
        continue;
      output += "  ";
      output += record;
      output += "\n";
    }
  }
  void print_base(std::string &output) { this->print_base(output, false); };

  void tuponope(){};

 protected:
  float somevar_{0.0};
};

}  // namespace demo22
}  // namespace esphome
