#pragma once
#include <Arduino.h>

class AdcReader {
public:
  explicit AdcReader(int pin) : _pin(pin) {}

  void begin() {
    pinMode(_pin, INPUT);
    // ESP32: analogReadResolution no siempre está disponible igual en todos los cores,
    // analogRead() ya retorna 0..4095 normalmente.
  }

  uint16_t readRaw() const {
    return static_cast<uint16_t>(analogRead(_pin));
  }

  float readNorm() const {
    // 12-bit típico: 0..4095
    const float raw = static_cast<float>(readRaw());
    float n = raw / 4095.0f;
    if (n < 0.0f) n = 0.0f;
    if (n > 1.0f) n = 1.0f;
    return n;
  }

private:
  int _pin;
};
