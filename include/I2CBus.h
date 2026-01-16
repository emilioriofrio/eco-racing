#pragma once
#include <Arduino.h>
#include <Wire.h>

class I2CBus {
public:
  I2CBus(TwoWire& wire, int sda, int scl, uint32_t freq_hz)
  : _wire(wire), _sda(sda), _scl(scl), _freq(freq_hz) {}

  void begin(bool internalPullups = false) {
    // Algunos cores respetan pullups internos con pinMode antes del begin
    if (internalPullups) {
      pinMode(_sda, INPUT_PULLUP);
      pinMode(_scl, INPUT_PULLUP);
    }
    _wire.begin(_sda, _scl, _freq);
  }

  TwoWire& wire() { return _wire; }

private:
  TwoWire& _wire;
  int _sda;
  int _scl;
  uint32_t _freq;
};
