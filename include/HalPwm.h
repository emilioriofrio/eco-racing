#pragma once
#include <Arduino.h>

class PwmOutput {
public:
  PwmOutput(int pin, int channel) : _pin(pin), _ch(channel) {}

  void begin(uint32_t freq_hz, uint8_t res_bits) {
    ledcSetup(_ch, freq_hz, res_bits);
    ledcAttachPin(_pin, _ch);
    _maxVal = (1U << res_bits) - 1U;
    setDuty(0.0f);
  }

  void setDuty(float duty01) {
    if (duty01 < 0.0f) duty01 = 0.0f;
    if (duty01 > 1.0f) duty01 = 1.0f;
    const uint32_t val = static_cast<uint32_t>(duty01 * static_cast<float>(_maxVal));
    ledcWrite(_ch, val);
    _lastDuty = duty01;
  }

  float lastDuty() const { return _lastDuty; }

private:
  int _pin;
  int _ch;
  uint32_t _maxVal = 1023;
  float _lastDuty = 0.0f;
};
