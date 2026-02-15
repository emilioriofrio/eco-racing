import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

CSV_PATH = "sim_out.csv"
DOWNSAMPLE = 20  # 20 muestras = 100 ms (si CONTROL_HZ=200)

df = pd.read_csv(CSV_PATH)
df["t_s"] = df["t_ms"] / 1000.0

# Reducir puntos para que sea liviano al graficar
dfp = df.iloc[::DOWNSAMPLE].copy()

# Elegir un instante de referencia (ajusta a gusto)
t0 = 35.0  # segundos
row = df.iloc[(df["t_s"] - t0).abs().argsort()[:1]].iloc[0]
duty_s = float(row["duty_solar"])
duty_e = float(row["duty_eolica"])

# Parámetros PWM
f_pwm = 20000.0
T = 1.0 / f_pwm
Vmax = 12.0

# Ventana de visualización
win_ms = 5.0
dt_us = 1.0
t = np.arange(0, win_ms/1000.0, dt_us*1e-6)

def pwm_wave(t, duty, Vmax, T):
  phase = np.mod(t, T)
  return np.where(phase < duty*T, Vmax, 0.0)

v_s = pwm_wave(t, duty_s, Vmax, T)
v_e = pwm_wave(t, duty_e, Vmax, T)

# 1) Entradas
plt.figure()
plt.plot(dfp["t_s"], dfp["solar_norm"], label="solar_norm")
plt.plot(dfp["t_s"], dfp["eolica_norm"], label="eolica_norm")
plt.xlabel("Tiempo (s)")
plt.ylabel("Entrada normalizada")
plt.title("Señales simuladas de entrada (Solar y Eólica)")
plt.grid(True)
plt.legend()

# 2) Salidas PWM (duty)
plt.figure()
plt.plot(dfp["t_s"], dfp["duty_solar"], label="duty_solar")
plt.plot(dfp["t_s"], dfp["duty_eolica"], label="duty_eolica")
plt.xlabel("Tiempo (s)")
plt.ylabel("Duty PWM")
plt.title("Salidas PWM (duty) por carril")
plt.grid(True)
plt.legend()

# 3) Entrada vs Duty (mismo carril) para ver clamp/seguimiento
plt.figure()
plt.plot(dfp["t_s"], dfp["solar_norm"], label="solar_norm")
plt.plot(dfp["t_s"], dfp["duty_solar"], label="duty_solar")
plt.xlabel("Tiempo (s)")
plt.ylabel("Valor normalizado / Duty")
plt.title("Carril Solar: entrada vs duty")
plt.grid(True)
plt.legend()

plt.figure()
plt.plot(dfp["t_s"], dfp["eolica_norm"], label="eolica_norm")
plt.plot(dfp["t_s"], dfp["duty_eolica"], label="duty_eolica")
plt.xlabel("Tiempo (s)")
plt.ylabel("Valor normalizado / Duty")
plt.title("Carril Eólico: entrada vs duty")
plt.grid(True)
plt.legend()

# 4) (Opcional) Distribución de duty para describir rangos de operación
plt.figure()
plt.hist(df["duty_solar"], bins=50, alpha=0.7, label="duty_solar")
plt.hist(df["duty_eolica"], bins=50, alpha=0.7, label="duty_eolica")
plt.xlabel("Duty PWM")
plt.ylabel("Frecuencia")
plt.title("Distribución de duty (histograma)")
plt.grid(True)
plt.legend()

# 5) PWM real (voltaje) vs tiempo en ventana corta alrededor de t0 para ver forma de onda
# PWM Solar
plt.figure()
plt.plot(t*1000.0, v_s)
plt.xlabel("Tiempo (ms)")
plt.ylabel("Voltaje (V)")
plt.title(f"PWM Solar (zoom {win_ms} ms) @ t≈{t0}s, duty={duty_s:.3f}")
plt.grid(True)

# PWM Eólica
plt.figure()
plt.plot(t*1000.0, v_e)
plt.xlabel("Tiempo (ms)")
plt.ylabel("Voltaje (V)")
plt.title(f"PWM Eólica (zoom {win_ms} ms) @ t≈{t0}s, duty={duty_e:.3f}")
plt.grid(True)


# 6) Vueltas acumuladas (laps) vs tiempo
plt.figure()
plt.plot(dfp["t_s"], dfp["laps_solar"], label="laps_solar")
plt.plot(dfp["t_s"], dfp["laps_eolica"], label="laps_eolica")
plt.xlabel("Tiempo (s)")
plt.ylabel("Vueltas acumuladas")
plt.title("Conteo de vueltas simulado (laps) vs tiempo")
plt.grid(True)
plt.legend()


plt.show()
