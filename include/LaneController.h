#pragma once
#include <Arduino.h>
#include "HalAdc.h"
#include "HalPwm.h"
#include "LapCounter.h"
#include "Telemetry.h"
#include "Config.h"

class LaneController {
public:
  LaneController(uint8_t id, AdcReader& adc, PwmOutput& pwm, LapCounter& laps)
  : _id(id), _adc(adc), _pwm(pwm), _laps(laps) {}

  void begin() {
    _state = LaneState::RUNNING;
  }

  void stop() {
    _pwm.setDuty(0.0f);
    _state = LaneState::FINISHED;
  }

  void faultStop() {
    _pwm.setDuty(0.0f);
    _state = LaneState::FAULT;
  }

  LaneTelemetry update() {
    LaneTelemetry t;
    t.state = _state;

    // Si está en FINISHED/FAULT, duty en 0
    if (_state == LaneState::FINISHED || _state == LaneState::FAULT) {
      t.duty = 0.0f;
      t.adc_norm = _adc.readNorm();
      t.laps = _laps.getLaps();
      return t;
    }

    // Control básico: duty proporcional a energía medida (ADC)
    const float adcNorm = _adc.readNorm();
    float duty = adcNorm;

    // Clamp de seguridad
    if (duty > MAX_DUTY) duty = MAX_DUTY;

    _pwm.setDuty(duty);

    t.adc_norm = adcNorm;
    t.duty = duty;
    t.laps = _laps.getLaps();
    t.state = _state;
    return t;
  }

  LaneState state() const { return _state; }

private:
  uint8_t _id;
  AdcReader& _adc;
  PwmOutput& _pwm;
  LapCounter& _laps;
  LaneState _state = LaneState::IDLE;
};
