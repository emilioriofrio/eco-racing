#include <Arduino.h>
#include <Wire.h>

#include "Config.h"
#include "Telemetry.h"
#include "HalAdc.h"
#include "HalPwm.h"
#include "LapCounter.h"
#include "I2CBus.h"
#include "Ina219Driver.h"
#include "OledDriver.h"
#include "LaneController.h"

// -------- Two I2C instances --------
TwoWire I2C_A(0);
TwoWire I2C_B(1);

// -------- Buses --------
I2CBus busA(I2C_A, I2C_A_SDA, I2C_A_SCL, I2C_FREQ_HZ);
I2CBus busB(I2C_B, I2C_B_SDA, I2C_B_SCL, I2C_FREQ_HZ);

// -------- HAL objects --------
AdcReader adcA(PIN_ADC_A);
AdcReader adcB(PIN_ADC_B);

PwmOutput pwmA(PIN_PWM_A, 0);
PwmOutput pwmB(PIN_PWM_B, 1);

LapCounter lapsA(PIN_IR_A, IR_DEBOUNCE_US);
LapCounter lapsB(PIN_IR_B, IR_DEBOUNCE_US);

// -------- Lane controllers --------
LaneController laneA(0, adcA, pwmA, lapsA);
LaneController laneB(1, adcB, pwmB, lapsB);

// -------- Devices per lane --------
Ina219Driver inaA(INA219_ADDR);
Ina219Driver inaB(INA219_ADDR);

OledDriver oledA(OLED_ADDR);
OledDriver oledB(OLED_ADDR);

// -------- Telemetry Queue --------
QueueHandle_t qTelemetry;

// -------- Shared packet --------
static TelemetryPacket lastPkt;

// -------- Simple race state --------
static bool finished = false;

static void controlTask(void* pv) {
  const TickType_t period = pdMS_TO_TICKS(1000 / CONTROL_HZ);
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    // Update lanes
    TelemetryPacket pkt;
    pkt.ms = millis();

    pkt.lane[0] = laneA.update();
    pkt.lane[1] = laneB.update();

    // Determinar ganador simple por laps
    if (!finished) {
      if (pkt.lane[0].laps >= TARGET_LAPS || pkt.lane[1].laps >= TARGET_LAPS) {
        finished = true;
        laneA.stop();
        laneB.stop();
      }
    }

    // Enviar telemetría (cola de 1 elemento; overwrite)
    xQueueOverwrite(qTelemetry, &pkt);

    vTaskDelayUntil(&lastWake, period);
  }
}

static void uiTask(void* pv) {
  const TickType_t period = pdMS_TO_TICKS(1000 / UI_HZ);
  TickType_t lastWake = xTaskGetTickCount();

  TelemetryPacket pkt;

  for (;;) {
    // Obtener última telemetría (si no hay, usa la previa)
    if (xQueuePeek(qTelemetry, &pkt, 0) == pdTRUE) {
      lastPkt = pkt;
    } else {
      pkt = lastPkt;
    }

    // Leer INA219 en cada carril (I2C independiente)
    pkt.lane[0].bus_v = inaA.busVoltage_V();
    pkt.lane[0].current_ma = inaA.current_mA();
    pkt.lane[0].power_mw = inaA.power_mW();

    pkt.lane[1].bus_v = inaB.busVoltage_V();
    pkt.lane[1].current_ma = inaB.current_mA();
    pkt.lane[1].power_mw = inaB.power_mW();

    // Render OLED por carril
    oledA.renderLane(0, pkt.lane[0].duty, pkt.lane[0].laps, pkt.lane[0].adc_norm,
                     pkt.lane[0].bus_v, pkt.lane[0].current_ma, pkt.lane[0].power_mw);

    oledB.renderLane(1, pkt.lane[1].duty, pkt.lane[1].laps, pkt.lane[1].adc_norm,
                     pkt.lane[1].bus_v, pkt.lane[1].current_ma, pkt.lane[1].power_mw);

    // Debug por serial (opcional)
    Serial.print("A laps="); Serial.print(pkt.lane[0].laps);
    Serial.print(" duty="); Serial.print(pkt.lane[0].duty, 3);
    Serial.print(" | B laps="); Serial.print(pkt.lane[1].laps);
    Serial.print(" duty="); Serial.println(pkt.lane[1].duty, 3);

    vTaskDelayUntil(&lastWake, period);
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // ADC
  adcA.begin();
  adcB.begin();

  // PWM
  pwmA.begin(PWM_FREQ_HZ, PWM_RES_BITS);
  pwmB.begin(PWM_FREQ_HZ, PWM_RES_BITS);

  // Lap counters (interrupts)
  lapsA.begin();
  lapsB.begin();

  // I2C buses
  // internalPullups=true solo para pruebas; en hardware real usa resistencias externas
  busA.begin(true);
  busB.begin(true);

  // INA219 begin per bus
  if (!inaA.begin(busA.wire())) {
    Serial.println("INA A not found");
  }
  if (!inaB.begin(busB.wire())) {
    Serial.println("INA B not found");
  }

  // OLED begin per bus
  if (!oledA.begin(busA.wire())) {
    Serial.println("OLED A not found");
  }
  if (!oledB.begin(busB.wire())) {
    Serial.println("OLED B not found");
  }

  // Lanes
  laneA.begin();
  laneB.begin();

  // Queue (1 elemento para overwrite)
  qTelemetry = xQueueCreate(1, sizeof(TelemetryPacket));

  // Crear tareas (dual core)
  // Core 1: control
  xTaskCreatePinnedToCore(controlTask, "ControlTask", 4096, nullptr, 3, nullptr, 1);

  // Core 0: UI/I2C
  xTaskCreatePinnedToCore(uiTask, "UiTask", 8192, nullptr, 1, nullptr, 0);

  Serial.println("System started");
}

void loop() {
  // Vacío. Todo corre en tareas FreeRTOS.
  vTaskDelay(pdMS_TO_TICKS(1000));
}
