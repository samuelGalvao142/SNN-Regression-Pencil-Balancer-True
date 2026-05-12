import pandas as pd
import matplotlib.pyplot as plt

# === CONFIG ===
csv_file = r"C:\Users\samue\PURDUE-2026\Pencil Balancer\PencilBalancingRobot\logs\log-8-05-06-2026\hough_cam1_cam1_hough.csv"  # coloque o caminho do seu arquivo aqui
bins = 200  # número de bins do histograma

# === LEITURA DO CSV ===
df = pd.read_csv(csv_file)

# Checagem básica (evita erro silencioso)
expected_cols = {"timestamp_us", "slope", "intercept"}
if not expected_cols.issubset(df.columns):
    raise ValueError(f"O CSV precisa conter as colunas: {expected_cols}")

# === EXTRAÇÃO DOS DADOS ===
slope = df["slope"].dropna()
intercept = df["intercept"].dropna()

# === HISTOGRAMA DO SLOPE ===
plt.figure()
plt.hist(slope, bins=bins)
plt.title("Histograma de Slope")
plt.xlabel("Slope")
plt.ylabel("Frequência")
plt.grid(True)

# === HISTOGRAMA DO INTERCEPT ===
plt.figure()
plt.hist(intercept, bins=bins)
plt.title("Histograma de Intercept")
plt.xlabel("Intercept")
plt.ylabel("Frequência")
plt.grid(True)

# === MOSTRAR ===
plt.show()