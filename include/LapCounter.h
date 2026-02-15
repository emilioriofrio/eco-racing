#pragma once

#if ECO_SIM
  #include <cstdint>
#else
  #include <Arduino.h>
  #include <xtensa/core-macros.h>
#endif

class LapCounter {
public:
  LapCounter(int pin, uint32_t debounce_us)
  : _pin(pin), _debounceUs(debounce_us) {}

  void begin() {
    _laps = 0;

#if !ECO_SIM
    pinMode(_pin, INPUT);
    _lastCcount = 0;
    attachInterruptArg(_pin, &LapCounter::isrThunk, this, RISING);
    _debounceCycles = (uint32_t)(_debounceUs * (uint32_t)getCpuFrequencyMhz());
#else
    _progress = 0.0f;
#endif
  }

  uint32_t getLaps() const { return _laps; }

  void reset() {
    _laps = 0;
#if ECO_SIM
    _progress = 0.0f;
#endif
  }

#if ECO_SIM
  // Modelo simple duty -> vueltas
  void tick(float duty, uint32_t dt_ms) {
    // k = “vueltas por segundo” si duty = 1
    const float k = 0.30f;           // Se puede calibrar para que el modelo sim se parezca a la realidad
    const float dt = (float)dt_ms * 0.001f;

    if (duty < 0.0f) duty = 0.0f;
    if (duty > 1.0f) duty = 1.0f;

    _progress += (k * duty) * dt;

    while (_progress >= 1.0f) {
      _laps++;
      _progress -= 1.0f;
    }
  }
#endif

private:
#if !ECO_SIM
  static void IRAM_ATTR isrThunk(void* arg) {
    LapCounter* self = (LapCounter*)arg;
    const uint32_t now = XTHAL_GET_CCOUNT();
    const uint32_t diff = now - self->_lastCcount;
    if (diff >= self->_debounceCycles) {
      self->_laps++;
      self->_lastCcount = now;
    }
  }
#endif

  int _pin;
  uint32_t _debounceUs;

  volatile uint32_t _laps = 0;

#if !ECO_SIM
  volatile uint32_t _lastCcount = 0;
  volatile uint32_t _debounceCycles = 0;
#else
  float _progress = 0.0f;
#endif
};
