#pragma once
#include <Arduino.h>
#include <xtensa/core-macros.h>  // XTHAL_GET_CCOUNT()

class LapCounter {
public:
  LapCounter(int pin, uint32_t debounce_us)
  : _pin(pin), _debounceUs(debounce_us) {}

  void begin() {
    pinMode(_pin, INPUT);
    _laps = 0;
    _lastCcount = 0;
    attachInterruptArg(_pin, &LapCounter::isrThunk, this, RISING);

    // Calcula umbral en ciclos (aprox): ciclos = us * (CPU_MHz)
    // getCpuFrequencyMhz() no se usa en ISR; lo precomputamos aquí.
    _debounceCycles = (uint32_t)(_debounceUs * (uint32_t)getCpuFrequencyMhz());
  }

  uint32_t getLaps() const { return _laps; }
  void reset() { _laps = 0; }

private:
  static void IRAM_ATTR isrThunk(void* arg) {
    LapCounter* self = (LapCounter*)arg;

    const uint32_t now = XTHAL_GET_CCOUNT();
    const uint32_t diff = now - self->_lastCcount;

    if (diff >= self->_debounceCycles) {
      self->_laps++;
      self->_lastCcount = now;
    }
  }

  int _pin;
  uint32_t _debounceUs;

  volatile uint32_t _laps = 0;
  volatile uint32_t _lastCcount = 0;
  volatile uint32_t _debounceCycles = 0;
};
