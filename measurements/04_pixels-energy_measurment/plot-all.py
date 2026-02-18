import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np
from pathlib import Path

current_dir = Path(__file__).resolve().parent

csv_files = list(current_dir.glob(f"*_measurements.csv"))

for csv_file in csv_files:
    df = pd.read_csv(csv_file)

    timestamp = int(csv_file.stem.split("_")[0])

    # df = df[0:1000]
   
    t = df["timestamp"] - df["timestamp"].iloc[0]
    df["energy_cumulative_J"] = df["energy_J"].cumsum()
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
    plt.savefig(f"{current_dir}/{timestamp}_plot.png")
    # plt.show()