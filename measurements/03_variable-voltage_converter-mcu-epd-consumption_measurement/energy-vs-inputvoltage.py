import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np
from pathlib import Path

current_dir = Path(__file__).resolve().parent

csv_files = list(current_dir.glob(f"*_measurements.csv"))

avg_voltage = []
total_energy = []

for csv_file in csv_files:
    df = pd.read_csv(csv_file)

    # Gemiddelde ingangsspanning
    v_mean = df["voltage_V"].mean()

    # Totale energie (per tijdsslot gesommeerd)
    e_total = df["energy_J"].sum()

    avg_voltage.append(v_mean)
    total_energy.append(e_total)

    print(f"{csv_file.name}: Vavg={v_mean:.3f} V, E={e_total:.6f} J")

# Plot
plt.figure()
plt.scatter(avg_voltage, total_energy)
plt.xlabel("Average Input Voltage [V]")
plt.ylabel("Total Energy [J]")
plt.title("Energy vs Average Input Voltage")
plt.grid(True)
plt.show()