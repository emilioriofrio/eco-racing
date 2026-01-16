#pragma once
#include <Arduino.h>
#include <Adafruit_INA219.h>
#include <Wire.h>

class Ina219Driver {
public:
  explicit Ina219Driver(uint8_t addr = 0x40) : _ina(addr) {}

  bool begin(TwoWire& w) {
    return _ina.begin(&w);
  }

  float busVoltage_V() { return _ina.getBusVoltage_V(); }
  float current_mA()   { return _ina.getCurrent_mA(); }
  float power_mW()     { return _ina.getPower_mW(); }

private:
  Adafruit_INA219 _ina;
};
