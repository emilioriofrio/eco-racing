#pragma once
#include <Arduino.h>

// Lane count 
static constexpr uint8_t LANES = 2;

// PWM pins
static constexpr int PIN_PWM_A = 25;
static constexpr int PIN_PWM_B = 26;

// IR sensors (interrupt)
static constexpr int PIN_IR_A = 34; // input only
static constexpr int PIN_IR_B = 35; // input only

// ADC pins (ADC1) 
static constexpr int PIN_ADC_A = 36; // SENSOR_VP
static constexpr int PIN_ADC_B = 39; // SENSOR_VN

// I2C buses (Uno por línea)
static constexpr int I2C_A_SDA = 21;
static constexpr int I2C_A_SCL = 22;

static constexpr int I2C_B_SDA = 16;
static constexpr int I2C_B_SCL = 17;

// I2C addresses
static constexpr uint8_t OLED_ADDR = 0x3C;
static constexpr uint8_t INA219_ADDR = 0x40;

// I2C freq 
static constexpr uint32_t I2C_FREQ_HZ = 400000;

// Control loop rates
static constexpr uint32_t CONTROL_HZ = 200;  // core 1
static constexpr uint32_t UI_HZ      = 10;   // core 0

// PWM config (LEDC)
static constexpr uint32_t PWM_FREQ_HZ = 20000;
static constexpr uint8_t  PWM_RES_BITS = 10;  // 0..1023
static constexpr float    MAX_DUTY = 0.85f;   // safety clamp

// Lap counting 
static constexpr uint32_t IR_DEBOUNCE_US = 3000; // ajustar

// Race config (cuántas vueltas es la meta)
static constexpr uint32_t TARGET_LAPS = 10; // ajustar
