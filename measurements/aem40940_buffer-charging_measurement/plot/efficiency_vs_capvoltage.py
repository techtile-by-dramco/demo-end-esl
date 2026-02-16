import pandas as pd
import os
import matplotlib.pyplot as plt
import numpy as np
import glob

from file_handler import *

cap_size_mf = 22

# file_name = "1731707775_868.0_-5.0_3.1"
# sub_path_name = "data_aem_33mf_with_return_loss"
sub_path_name = f"data_aem_{cap_size_mf}mf_with_return_loss"

freq_buffer = [868.0, 920.0]
pwr_buffer = np.linspace(-5, 15, 9) #"-5.0"
target_voltage = "3.1"

freq = 868.0
pwr = 10.0

# Get panda file
data = get_panda_file(sub_path_name, freq, pwr, target_voltage)

# Filter the dataset to only include rows where voltage_uV is below 2e6
# filtered_data = data[data["voltage_uV"] < 0.5e6].
filtered_data = data

# Example: Access individual columns
timestamps = filtered_data["timestamp"]
voltages = filtered_data["voltage_uV"]
reflected_powers = filtered_data["reflected_power_dbm"]

t = np.asarray(timestamps)
v = np.asarray(voltages)/1e6

print(f"Time begin: {t[0]}")
print(f"Time total: {t[-1]}")

v_range = 0.125

bins = np.arange(0, 3.125, v_range)

print(f"Total samples: {len(t)}")

# print(bins)

bin_indices = np.digitize(v, bins)

# Enforce monotonic bin assignment

bin_indices = np.maximum.accumulate(bin_indices)

print(bin_indices)

print(f": {len(bin_indices)}")


# print(len(bin_indices))

# print(bin_indices)
# print(v[bin_indices == 1])

efficiency = []
time_intervals = []

samples_handled = 0

for bin in range(1, len(bins) + 1):
    time_bin_values = t[bin_indices == bin]
    voltage_bin_values = v[bin_indices == bin]

    samples_handled = samples_handled + len(time_bin_values)

    print("---")

    # print(len(time_bin_values))
    # print(len(voltage_bin_values))

    # print(10**(pwr/10))
    input_pwr_mW = 10**(pwr/10) 
    ie = input_pwr_mW/1e3*(time_bin_values[-1]-time_bin_values[0])

    # print(ie)

    se = 0

    for i in range(len(voltage_bin_values)-1):
        se = se + (voltage_bin_values[i + 1]**2 - voltage_bin_values[i]**2)*cap_size_mf*1e-3/2

    eff = se/ie*100

    time_interval = time_bin_values[-1]-time_bin_values[0]

    print(f"Input energy: {ie}")
    print(f"Stored energy: {se}")
    print(f"Stored energy theo: {((v_range*bin)**2 - (v_range*(bin-1))**2)*cap_size_mf*1e-3/2}")
    print(f"Time to reach {v_range*bin}V: {time_interval}")
    print(f"Efficiency: {eff}")
    print(f"Begin time: {time_bin_values[0]}")
    print(f"End time: {time_bin_values[-1]}")

    efficiency.append(eff)
    time_intervals.append(time_interval)


print(samples_handled)

# print(efficiency)

print(np.sum(time_intervals))

tt = 0
for tim in time_intervals:
    tt = tt + tim

print(f"t: {tt}")


# Calculate bin centers for bar plot
bin_centers = (bins[:-1] + bins[1:]) / 2

fig, ax1 = plt.subplots()

# Create bar plot
ax1.bar(bin_centers, efficiency[:len(bins)-1], width=v_range, edgecolor='black', alpha=0.7)

ax1.set_xlabel("Capacitor voltage [V]")
ax1.set_ylabel("Charge efficiency [%]")


# plt.show()

# # Create bar plot
# plt.bar(bin_centers, time_intervals[:len(bins)-1], width=v_range, edgecolor='black', alpha=0.7)

# plt.xlabel("Voltage [V]")
# plt.ylabel("Charge time [s]")


# plt.show()
# exit()


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

tikzplotlib.save(f"{os.getcwd()}/plot/images/charge_efficiency_{pwr}_{cap_size_mf}.tex")