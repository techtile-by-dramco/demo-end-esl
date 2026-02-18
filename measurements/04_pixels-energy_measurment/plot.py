import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np

TIME = 1771431566

# Path to save the experiment data as a YAML file
current_file_path = os.path.abspath(__file__)
current_dir = os.path.dirname(current_file_path)
parent_path = os.path.dirname(current_dir)
filename = os.path.basename(current_dir)

file_path = f"{current_dir}/{TIME}_measurements.csv"

# CSV inlezen
df = pd.read_csv(file_path)

# Tijd normaliseren (start = 0 s)
t = df["timestamp"] - df["timestamp"].iloc[0]

# Cumulative energy in Joule
df["energy_cumulative_J"] = df["energy_J"].cumsum()

# Total energy [1]
print(np.sum(df["energy_J"]))

# Total energy [2]
print(df["energy_cumulative_J"].iloc[-1])

# Total energy [3]
print(np.sum(df["power_W"][:-1]*(df["timestamp"].diff())/1e3))

import matplotlib.pyplot as plt

fig, axs = plt.subplots(4, 1, sharex=True, figsize=(10, 8))

# 1️⃣ Current
axs[0].plot(t, df["current_A"])
axs[0].set_ylabel("Current [A]")
axs[0].set_title("Current")

# 2️⃣ Voltage
axs[1].plot(t, df["voltage_V"])
axs[1].set_ylabel("Voltage [V]")
axs[1].set_title("Voltage")

# 3️⃣ Power
axs[2].plot(t, df["power_W"])
axs[2].set_ylabel("Power [W]")
axs[2].set_title("Power")

# 4️⃣ Energy (cumulative)
axs[3].plot(t, df["energy_cumulative_J"])
axs[3].set_ylabel("Energy [J]")
axs[3].set_xlabel("Time [s]")
axs[3].set_title("Cumulative Energy")

for ax in axs:
    ax.grid(True)

fig.suptitle(f"Average supply voltage {round(np.average(df["voltage_V"]),2)}V", fontsize=14)
fig.tight_layout()
plt.show()
