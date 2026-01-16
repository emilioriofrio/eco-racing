#pragma once
#include <stdint.h>

enum class LaneState : uint8_t {
  IDLE = 0,
  RUNNING,
  FINISHED,
  FAULT
};

struct LaneTelemetry {
  float adc_norm = 0.0f;     // 0..1 desde ADC
  float duty = 0.0f;         // 0..1 aplicado
  uint32_t laps = 0;
  LaneState state = LaneState::IDLE;

  // Se llenan en la tarea UI (INA219)
  float bus_v = 0.0f;
  float current_ma = 0.0f;
  float power_mw = 0.0f;
};

struct TelemetryPacket {
  LaneTelemetry lane[2];
  uint32_t ms = 0;
};
