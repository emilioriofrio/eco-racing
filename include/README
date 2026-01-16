# 1) ECO-RACING ESP32 (2 carriles) — PlatformIO + POO + FreeRTOS (Dual Core)

Proyecto base para control de 2 carriles (2 carros) usando ESP32-WROOM-32D, con:
- 2 salidas PWM (una por motor)
- 2 sensores IR de vueltas por interrupción (uno por carril)
- 2 entradas analógicas (energía por carril) en ADC1
- 2 buses I2C separados (uno por carril) para evitar conflictos:
  - 1 OLED por carril (SSD1306)
  - 1 INA219 por carril

La arquitectura separa:
- Control rápido (core 1) -> ADC + control PWM + vueltas
- UI/I2C lento (core 0) -> INA219 + OLED + telemetría

---

## 2) Dependencias (PlatformIO)

En `platformio.ini`:

- Adafruit GFX Library
- Adafruit SSD1306
- Adafruit INA219

---

## 3) Pines y configuración (Config.h)

Archivo: `include/Config.h`

### Objetivo
Centraliza parámetros de hardware, tasas de actualización y constantes de control.

### Constantes importantes

#### Cantidad de carriles
- `LANES`

#### Pines de PWM (motores)
- `PIN_PWM_A`
- `PIN_PWM_B`

#### Pines de sensores IR (interrupción)
- `PIN_IR_A` (entrada)
- `PIN_IR_B` (entrada)

#### Pines ADC (energía por carril)
- `PIN_ADC_A` (ADC1)
- `PIN_ADC_B` (ADC1)

#### Buses I2C por carril
- Carril A: `I2C_A_SDA`, `I2C_A_SCL`
- Carril B: `I2C_B_SDA`, `I2C_B_SCL`

#### Direcciones I2C típicas
- `OLED_ADDR`
- `INA219_ADDR`

#### Frecuencias
- `I2C_FREQ_HZ`
- `CONTROL_HZ`
- `UI_HZ`

#### Config PWM (LEDC)
- `PWM_FREQ_HZ`
- `PWM_RES_BITS`
- `MAX_DUTY` (límite de seguridad)

#### Anti-rebote IR
- `IR_DEBOUNCE_US`

#### Lógica de carrera
- `TARGET_LAPS`

---

## 4) Tipos de telemetría (Telemetry.h)

Archivo: `include/Telemetry.h`

### Objetivo
Define estructuras de datos compartidas entre tareas (control/UI) y estados.

### Tipos

#### `enum class LaneState`
Estados del carril:
- `IDLE`
- `RUNNING`
- `FINISHED`
- `FAULT`

#### `struct LaneTelemetry`
Paquete de datos por carril.

**Variables**
- `float adc_norm`  
  Valor normalizado 0..1 del ADC del carril.
- `float duty`  
  Duty 0..1 aplicado al PWM.
- `uint32_t laps`  
  Vueltas contadas por el sensor IR.
- `LaneState state`  
  Estado lógico del carril.
- `float bus_v`  
  (UI) Voltaje del bus medido por INA219.
- `float current_ma`  
  (UI) Corriente medida por INA219.
- `float power_mw`  
  (UI) Potencia medida por INA219.

#### `struct TelemetryPacket`
Paquete global (2 carriles).

**Variables**
- `LaneTelemetry lane[2]`  
  Telemetría de carril A y B.
- `uint32_t ms`  
  Timestamp (millis).

---

## 5) Lectura ADC (HalAdc.h) — clase `AdcReader`

Archivo: `include/HalAdc.h`

### Responsabilidad
Encapsular la lectura analógica del carril (energía disponible) y entregar el valor crudo o normalizado.

### Variables privadas
- `int _pin`  
  GPIO de ADC asociado al canal.

### Funciones públicas
- `AdcReader(int pin)`  
  Constructor, almacena el pin ADC.
- `void begin()`  
  Configura el pin como entrada.
- `uint16_t readRaw() const`  
  Retorna `analogRead(_pin)` como entero (típicamente 0..4095).
- `float readNorm() const`  
  Retorna valor normalizado 0..1 (clamped).

---

## 6) Salida PWM (HalPwm.h) — clase `PwmOutput`

Archivo: `include/HalPwm.h`

### Responsabilidad
Encapsular el PWM usando LEDC de ESP32.

### Variables privadas
- `int _pin`  
  GPIO de salida PWM.
- `int _ch`  
  Canal LEDC asignado.
- `uint32_t _maxVal`  
  Valor máximo según resolución (`2^res - 1`).
- `float _lastDuty`  
  Último duty aplicado (0..1).

### Funciones públicas
- `PwmOutput(int pin, int channel)`  
  Constructor, define pin y canal.
- `void begin(uint32_t freq_hz, uint8_t res_bits)`  
  Configura LEDC (`ledcSetup`, `ledcAttachPin`) y calcula `_maxVal`.
- `void setDuty(float duty01)`  
  Clampa 0..1, convierte a rango LEDC y hace `ledcWrite`.
- `float lastDuty() const`  
  Retorna el último duty aplicado.

---

## 7) Conteo de vueltas (LapCounter.h) — clase `LapCounter`

Archivo: `include/LapCounter.h`

### Responsabilidad
Contar pulsos de un sensor IR por interrupción para no perder eventos, con anti-rebote por tiempo.

### Variables privadas
- `int _pin`  
  GPIO del sensor IR.
- `uint32_t _debounceUs`  
  Ventana mínima entre pulsos válidos (microsegundos).
- `volatile uint32_t _laps`  
  Contador de pulsos/vueltas (se actualiza en ISR).
- `volatile uint32_t _lastUs`  
  Timestamp `micros()` del último pulso válido.

### Funciones públicas
- `LapCounter(int pin, uint32_t debounce_us)`  
  Constructor: pin + anti-rebote.
- `void begin()`  
  Configura pin y llama `attachInterruptArg` con ISR.
- `uint32_t getLaps() const`  
  Retorna `_laps`.
- `void reset()`  
  Reinicia `_laps`.

### Funciones privadas (ISR)
- `static void IRAM_ATTR isrThunk(void* arg)`  
  Función puente para llamar a método de instancia.
- `void IRAM_ATTR handleISR()`  
  Aplica anti-rebote por tiempo y aumenta `_laps`.

---

## 8) Bus I2C por carril (I2CBus.h) — clase `I2CBus`

Archivo: `include/I2CBus.h`

### Responsabilidad
Encapsular un `TwoWire` con pines SDA/SCL y frecuencia fija, para tener un bus por carril.

### Variables privadas
- `TwoWire& _wire`  
  Instancia `TwoWire` asociada (I2C0 o I2C1).
- `int _sda`, `int _scl`  
  Pines SDA y SCL.
- `uint32_t _freq`  
  Frecuencia del bus.

### Funciones públicas
- `I2CBus(TwoWire& wire, int sda, int scl, uint32_t freq_hz)`  
  Constructor: referencia a bus + pines + frecuencia.
- `void begin(bool internalPullups = false)`  
  Inicializa el bus. Si `internalPullups=true`, aplica `INPUT_PULLUP` (solo para pruebas; recomendado usar pull-ups externos en hardware real).
- `TwoWire& wire()`  
  Retorna la referencia a `TwoWire` para pasarlo a drivers.

---

## 9) Medición de potencia (Ina219Driver.h) — clase `Ina219Driver`

Archivo: `include/Ina219Driver.h`

### Responsabilidad
Driver simple para INA219 (voltaje/corriente/potencia) usando Adafruit INA219.

### Variables privadas
- `Adafruit_INA219 _ina`  
  Objeto del driver con dirección I2C.

### Funciones públicas
- `Ina219Driver(uint8_t addr = 0x40)`  
  Constructor con dirección.
- `bool begin(TwoWire& w)`  
  Inicializa INA219 en el bus indicado.
- `float busVoltage_V()`  
  Retorna el voltaje del bus.
- `float current_mA()`  
  Retorna la corriente.
- `float power_mW()`  
  Retorna potencia.

---

## 10) Pantalla OLED (OledDriver.h) — clase `OledDriver`

Archivo: `include/OledDriver.h`

### Responsabilidad
Mostrar telemetría del carril en OLED SSD1306 (128x64) usando Adafruit SSD1306.

### Variables privadas
- `uint8_t _addr`  
  Dirección I2C de la OLED.
- `Adafruit_SSD1306 _disp`  
  Instancia del display (se construye con el `TwoWire` del carril).

### Funciones públicas
- `OledDriver(uint8_t addr, int width=128, int height=64)`  
  Constructor del driver.
- `bool begin(TwoWire& w)`  
  Inicializa el display sobre el bus del carril, limpia pantalla y configura texto.
- `void renderLane(...)`  
  Renderiza una vista simple:
  - índice de carril
  - duty
  - ADC normalizado
  - vueltas
  - V/I/P desde INA219

---

## 11) Control por carril (LaneController.h) — clase `LaneController`

Archivo: `include/LaneController.h`

### Responsabilidad
Implementar el control del carril:
- Lee energía (ADC)
- Calcula duty PWM (control básico proporcional)
- Aplica límites de seguridad (`MAX_DUTY`)
- Provee telemetría por carril
- Respeta estados (RUNNING/FINISHED/FAULT)

### Variables privadas
- `uint8_t _id`  
  ID de carril.
- `AdcReader& _adc`  
  Referencia al ADC del carril.
- `PwmOutput& _pwm`  
  Referencia al PWM del carril.
- `LapCounter& _laps`  
  Referencia al contador de vueltas.
- `LaneState _state`  
  Estado del carril.

### Funciones públicas
- `LaneController(uint8_t id, AdcReader& adc, PwmOutput& pwm, LapCounter& laps)`  
  Constructor: inyección de dependencias.
- `void begin()`  
  Pasa el carril a `RUNNING`.
- `void stop()`  
  Duty a 0 y estado `FINISHED`.
- `void faultStop()`  
  Duty a 0 y estado `FAULT`.
- `LaneTelemetry update()`  
  Función principal del carril:
  - Si `FINISHED` o `FAULT`: duty=0.
  - Si `RUNNING`: duty = `adc_norm` con clamp a `MAX_DUTY`.
  - Retorna telemetría actual (adc_norm, duty, laps, state).
- `LaneState state() const`  
  Retorna estado actual.

---

## 12) main.cpp

Archivo: `src/main.cpp`

### Objetivo
Crear y conectar todos los objetos del sistema, inicializar buses y lanzar tareas FreeRTOS en 2 núcleos.

### Objetos principales
- `TwoWire I2C_A(0)`, `TwoWire I2C_B(1)`  
  Instancias de buses I2C.
- `I2CBus busA(...)`, `I2CBus busB(...)`  
  Configuración por carril.
- `AdcReader adcA`, `adcB`
- `PwmOutput pwmA`, `pwmB`
- `LapCounter lapsA`, `lapsB`
- `LaneController laneA`, `laneB`
- `Ina219Driver inaA`, `inaB`
- `OledDriver oledA`, `oledB`
- `QueueHandle_t qTelemetry`  
  Cola FreeRTOS (1 elemento) para telemetría.

### Tareas FreeRTOS

#### `controlTask` (Core 1)
Frecuencia: `CONTROL_HZ`

Acciones:
- llama `laneA.update()` y `laneB.update()`
- determina fin de carrera (si alguna llega a `TARGET_LAPS`)
- envía telemetría con `xQueueOverwrite(qTelemetry, &pkt)`

#### `uiTask` (Core 0)
Frecuencia: `UI_HZ`

Acciones:
- lee último `TelemetryPacket` de la cola con `xQueuePeek`
- lee INA219 por carril (cada uno en su bus)
- actualiza OLED por carril
- imprime debug por serial (opcional)

### `setup()`
- Inicializa ADC, PWM, LapCounter (ISR), I2C buses
- Inicializa INA219 y OLED por carril
- Inicializa carriles
- Crea cola de telemetría
- Crea tareas fijadas a núcleo:
  - `controlTask` -> core 1
  - `uiTask` -> core 0

### `loop()`
Vacío (el sistema corre por tareas FreeRTOS).

---

## 13) Notas importantes de hardware

### Pull-ups I2C
Aunque se habiliten pull-ups internos para pruebas, para hardware real se recomienda:
- Pull-up externo en SDA y SCL por bus (ej: 4.7 kΩ a 3.3 V).

### Direcciones I2C
Al usar dos buses I2C separados (uno por carril), se pueden repetir direcciones:
- OLED A y OLED B pueden ser 0x3C
- INA219 A y INA219 B pueden ser 0x40