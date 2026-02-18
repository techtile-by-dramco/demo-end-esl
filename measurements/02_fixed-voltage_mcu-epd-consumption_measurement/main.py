import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os

# Load CSV files
voltage_df = pd.read_csv('voltage.csv')
current_df = pd.read_csv('current.csv')[::4]

# Convert to numpy buffers
timestamp_buffer = voltage_df['Timestamp'].values
voltage_buffer = voltage_df['Value'].values
current_buffer = current_df['Value'].values

# Find start and stop index and update buffers (remove samples)
start_index = 0
end_index = 0

# Find start
for i,volt in enumerate(voltage_buffer):
    if volt > 2.5:
      start_index = i
      break

# Find end
for i,volt in enumerate(voltage_buffer):
    if volt > 2.5:
      end_index = i

timestamp_buffer = timestamp_buffer[start_index:end_index] - timestamp_buffer[start_index]
voltage_buffer = voltage_buffer[start_index:end_index]
current_buffer = current_buffer[start_index:end_index]

# Sample rate
sample_rate = int(len(timestamp_buffer)/timestamp_buffer[-1])
# print(sample_rate)

# Manual restrictions (remove samples)
time_restriction = 5.2 #seconds
restriction = int(time_restriction*sample_rate)

timestamp_buffer = timestamp_buffer[:restriction]
voltage_buffer = voltage_buffer[:restriction]
current_buffer = current_buffer[:restriction]*1000

# Calculate energy using trapezoidal rule
delta_t = timestamp_buffer[1:] - timestamp_buffer[:-1]  # Time intervals
power = (voltage_buffer[:-1] * current_buffer[:-1] + voltage_buffer[1:] * current_buffer[1:]) / 2  # Average power
energy = np.sum(power * delta_t)

print(f"Energy: {energy*1e3} mJ")

fig, ax1 = plt.subplots(figsize=(10, 6))
ax2 = ax1.twinx()
ax1.plot(timestamp_buffer, voltage_buffer, label='Voltage [V]')#, color='blue')
ax1.legend()
ax2.plot(timestamp_buffer, current_buffer, label='Current [mA]', color='C1')# linestyle='--')
ax2.set_ylim(0,np.mean(current_buffer)*2)
ax2.legend()
ax1.set_xlabel("Time [s]")
ax2.set_ylabel("Current [mA]")
ax1.set_ylabel("Voltage [V]")
plt.savefig("epd-mcu-energy-consumption.png")
plt.show()

TIKZPLOTLIB = False

if TIKZPLOTLIB:
    import tikzplotlib

    def tikzplotlib_fix_ncols(obj):
        """
        workaround for matplotlib 3.6 renamed legend's _ncol to _ncols, which breaks tikzplotlib
        """
        if hasattr(obj, "_ncols"):
            obj._ncol = obj._ncols
        for child in obj.get_children():
            tikzplotlib_fix_ncols(child)

    tikzplotlib_fix_ncols(fig)

    tikzplotlib.save(f"{os.getcwd()}/images/energy_consumption.tex")