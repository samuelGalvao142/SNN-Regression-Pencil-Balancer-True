import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# ========================
# CONFIG
# ========================
file_test = r"C:\Users\pcadm\Downloads\SNN\SNN-Regression-Pencil-Balancer-True\tests\snntest.csv"
file_ref  = r"C:\Users\pcadm\Downloads\SNN\SNN-Regression-Pencil-Balancer-True\Dataset\hough_cam1_cam1_hough.csv"

slice_test = None   # ex: (0, 5000)
slice_ref  = None

# latência média da SNN (3–4 ms → usar 3.5 ms)
snn_dt_us = 3500  # microsegundos

tolerance = 6000   # 6 ms

# ========================
# LOAD
# ========================
df_test = pd.read_csv(file_test)
df_ref  = pd.read_csv(file_ref)

# ========================
# DESNORMALIZAÇÃO (TESTE)
# ========================
df_test["lin_m"] = (df_test["lin_m"] * 1.76e6) - 0.44e6
df_test["lin_b"] = (df_test["lin_b"] * 44000) - 27500

# ========================
# PADRONIZAR TEMPO
# ========================
df_test = df_test.rename(columns={"timestep": "index"})
df_ref  = df_ref.rename(columns={"timestamp_us": "time"})

df_ref = df_ref.sort_values("time")

# ========================
# RECONSTRUIR TEMPO DA SNN
# ========================
start_time = df_ref["time"].min()

df_test["time"] = start_time + np.arange(len(df_test)) * snn_dt_us

# ========================
# SLICING
# ========================
if slice_test:
    df_test = df_test.iloc[slice_test[0]:slice_test[1]]

if slice_ref:
    df_ref = df_ref.iloc[slice_ref[0]:slice_ref[1]]

# ========================
# DEBUG
# ========================
print("Test range:", df_test["time"].min(), df_test["time"].max())
print("Ref  range:", df_ref["time"].min(), df_ref["time"].max())

# ========================
# ALIGNMENT (ROBUSTO)
# ========================
df = pd.merge_asof(
    df_test.sort_values("time"),
    df_ref.sort_values("time"),
    on="time",
    direction="nearest",
    tolerance=tolerance,
    suffixes=("_test", "_ref")
)

print("Antes dropna:", len(df))
df = df.dropna()
print("Depois dropna:", len(df))

# ========================
# MÉTRICAS
# ========================
df["diff_m"] = df["lin_m_test"] - df["lin_m_ref"]
df["diff_b"] = df["lin_b_test"] - df["lin_b_ref"]

mae_m = df["diff_m"].abs().mean()
mae_b = df["diff_b"].abs().mean()

rmse_m = np.sqrt((df["diff_m"]**2).mean())
rmse_b = np.sqrt((df["diff_b"]**2).mean())

print("\n===== RESULTADOS =====")
print(f"MAE  lin_m: {mae_m}")
print(f"MAE  lin_b: {mae_b}")
print(f"RMSE lin_m: {rmse_m}")
print(f"RMSE lin_b: {rmse_b}")

# ========================
# PLOTS
# ========================

# lin_m
plt.figure()
plt.plot(df_test["time"], df_test["lin_m"], label="SNN lin_m", alpha=0.7)
plt.plot(df_ref["time"], df_ref["lin_m"], label="GT lin_m", alpha=0.7)
plt.title("lin_m: SNN vs Ground Truth")
plt.legend()
plt.grid()

# lin_b
plt.figure()
plt.plot(df_test["time"], df_test["lin_b"], label="SNN lin_b", alpha=0.7)
plt.plot(df_ref["time"], df_ref["lin_b"], label="GT lin_b", alpha=0.7)
plt.title("lin_b: SNN vs Ground Truth")
plt.legend()
plt.grid()

# erro
plt.figure()
plt.plot(df["time"], df["diff_m"], label="erro lin_m")
plt.plot(df["time"], df["diff_b"], label="erro lin_b")
plt.title("Erro ao longo do tempo")
plt.legend()
plt.grid()

plt.show()