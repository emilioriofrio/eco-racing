#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class OledDriver {
public:
  OledDriver(uint8_t addr, int width = 128, int height = 64)
  : _addr(addr), _disp(width, height, &Wire, -1) {}

  // Nota: el constructor anterior usa &Wire por defecto.
  // Para usar un TwoWire distinto, re-inicializamos con begin(wire).
  bool begin(TwoWire& w) {
    _disp = Adafruit_SSD1306(128, 64, &w, -1);
    if (!_disp.begin(SSD1306_SWITCHCAPVCC, _addr)) {
      return false;
    }
    _disp.clearDisplay();
    _disp.setTextSize(1);
    _disp.setTextColor(SSD1306_WHITE);
    _disp.display();
    return true;
  }

  void renderLane(uint8_t laneIndex,
                  float duty,
                  uint32_t laps,
                  float adcNorm,
                  float busV,
                  float curmA,
                  float powmW) {
    _disp.clearDisplay();
    _disp.setCursor(0, 0);
    _disp.print("Lane ");
    _disp.println(laneIndex);

    _disp.print("Duty: "); _disp.println(duty, 3);
    _disp.print("ADC : "); _disp.println(adcNorm, 3);
    _disp.print("Laps: "); _disp.println(laps);

    _disp.print("Vbus: "); _disp.print(busV, 2); _disp.println(" V");
    _disp.print("I   : "); _disp.print(curmA, 1); _disp.println(" mA");
    _disp.print("P   : "); _disp.print(powmW, 1); _disp.println(" mW");

    _disp.display();
  }

private:
  uint8_t _addr;
  Adafruit_SSD1306 _disp;
};
