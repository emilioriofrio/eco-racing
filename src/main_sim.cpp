#if ECO_SIM

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdint>

#include "Config.h"
#include "HalAdc.h"
#include "HalPwm.h"

static constexpr uint32_t SIM_MS_TOTAL = 60000;             // 60 s
static constexpr uint32_t DT_MS = 1000 / CONTROL_HZ;        // 5 ms si CONTROL_HZ=200

static float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

float progA = 0.0f, progB = 0.0f;
uint32_t lapsA = 0, lapsB = 0;
const float k = 0.20f; // vueltas/seg cuando duty=1 

int main() {

  bool finished = false;
  const uint32_t RESET_MS = 50000; // Reset ocurre a los 50 segundos para probar la lógica de reinicio
  bool reset_evt = false;
  
  // “Lectores” ADC (en SIM deben devolver norm 0..1 sin Arduino)
  AdcReader adcSolar(PIN_ADC_A);
  AdcReader adcEolica(PIN_ADC_B);

  // PWM virtual: solo guardará duty (en SIM)
  PwmOutput pwmSolar(PIN_PWM_A, 0);
  PwmOutput pwmEolica(PIN_PWM_B, 1);

  adcSolar.begin();
  adcEolica.begin();
  pwmSolar.begin(PWM_FREQ_HZ, PWM_RES_BITS);
  pwmEolica.begin(PWM_FREQ_HZ, PWM_RES_BITS);

  std::ofstream f("sim_out.csv", std::ios::out | std::ios::trunc);
  if (!f.is_open()) {
    std::cerr << "No se pudo crear sim_out.csv\n";
    return 1;
  }

  f << "t_ms,solar_norm,eolica_norm,duty_solar,duty_eolica,laps_solar,laps_eolica,finished,reset_evt\n";
  f << std::fixed << std::setprecision(6);

  for (uint32_t t = 0; t <= SIM_MS_TOTAL; t += DT_MS) {

    reset_evt = (t == RESET_MS);
    if (reset_evt) {
      finished = false;
      progA = progB = 0.0f;
      lapsA = lapsB = 0;
    }

    // Para que HalAdc (SIM) sepa el tiempo actual
    AdcReader::setTimeMs(t);

    const float solar = clamp01(adcSolar.readNorm());
    const float eolica = clamp01(adcEolica.readNorm());

    // Control más simple: duty = señal (con saturación MAX_DUTY)
    float dutyS = 0.0f;
    float dutyE = 0.0f;

    if (!finished) {
      dutyS = solar;
      dutyE = eolica;

      if (dutyS > MAX_DUTY) dutyS = MAX_DUTY;
      if (dutyE > MAX_DUTY) dutyE = MAX_DUTY;
    }

    pwmSolar.setDuty(dutyS);
    pwmEolica.setDuty(dutyE);

    if (!finished) {
      const float dt_s = (float)DT_MS * 0.001f;

      progA += k * pwmSolar.lastDuty() * dt_s;
      progB += k * pwmEolica.lastDuty() * dt_s;

      while (progA >= 1.0f) { lapsA++; progA -= 1.0f; }
      while (progB >= 1.0f) { lapsB++; progB -= 1.0f; }

      if (lapsA >= TARGET_LAPS || lapsB >= TARGET_LAPS) {
        finished = true;
        pwmSolar.setDuty(0.0f);
        pwmEolica.setDuty(0.0f);
      }
    }


    f << t << ","
      << solar << ","
      << eolica << ","
      << pwmSolar.lastDuty() << ","
      << pwmEolica.lastDuty() << ","
      << lapsA << ","
      << lapsB << ","
      << (finished ? 1 : 0) << ","
      << (reset_evt ? 1 : 0)
      << "\n";
  }

  f.close();
  std::cout << "OK: sim_out.csv generado.\n";
  return 0;
}

#endif
