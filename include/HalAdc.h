#pragma once

#if ECO_SIM
  #include <cstdint>
  #include <cmath> 
#else
  #include <Arduino.h>
#endif

class AdcReader {
public:
  explicit AdcReader(int pin) : _pin(pin) {}

  void begin() {}

  uint16_t readRaw() const {
#if ECO_SIM
    // raw virtual 0..4095
    return (uint16_t)(_lastNorm * 4095.0f);
#else
    return static_cast<uint16_t>(analogRead(_pin));
#endif
  }

  float readNorm() const {
#if ECO_SIM
    // Generador de señal según “canal” (pin)
    // Solar: rampa + nubes
    // Eólica: ráfagas
    float v = signal01(_pin, _t_ms);
    _lastNorm = clamp01(v);
    return _lastNorm;
#else
    const float raw = static_cast<float>(readRaw());
    float n = raw / 4095.0f;
    if (n < 0.0f) n = 0.0f;
    if (n > 1.0f) n = 1.0f;
    return n;
#endif
  }

#if ECO_SIM
  // Sim: el main actualiza el tiempo
  static void setTimeMs(uint32_t t) { _t_ms = t; }
#endif

private:
  int _pin;

#if ECO_SIM
  inline static uint32_t _t_ms = 0;
  mutable float _lastNorm = 0.0f;

  static float clamp01(float x) {
    if (x < 0.0f) return 0.0f;
    if (x > 1.0f) return 1.0f;
    return x;
  }

  static float signal01(int pin, uint32_t t_ms) {
    // Selección: PIN_ADC_A = solar, PIN_ADC_B = eólica
    // Esto usa el número de pin como identificador.

    // Normaliza tiempo a segundos
    const float t = (float)t_ms * 0.001f;

    // Solar (PIN_ADC_A=36): rampa lenta + “nubes” (caídas cortas)
    if (pin == 36) {
      float ramp = t / 30.0f;            // 0..1 en 30 s
      if (ramp > 1.0f) ramp = 1.0f;

      // nubes: cada ~7s baja por 0.8s
      float cloud = 1.0f;
      float phase = fmodf(t, 7.0f);
      if (phase < 0.8f) cloud = 0.55f;

      return ramp * cloud;
    }

    // Eólica (PIN_ADC_B=39): base + ráfagas
    if (pin == 39) {
      float base = 0.25f;
      // ráfaga: cada 5s dura 1s
      float phase = fmodf(t, 5.0f);
      float gust = (phase < 1.0f) ? 0.55f : 0.0f;
      return base + gust;
    }

    // Default
    return 0.0f;
  }
#endif
};
