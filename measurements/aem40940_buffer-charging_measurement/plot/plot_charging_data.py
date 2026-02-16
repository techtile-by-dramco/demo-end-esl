import pandas as pd
import os
import matplotlib.pyplot as plt
import numpy as np
import glob

from file_handler import *

# --- SETTINGS --- #
harvester_type = "aem" #nxp
cap_size_mf = 22
# ---------------- #

# ---------------- #
if harvester_type == "aem":
    meas_voltage = "3.1"
    start_pwr = -7.5
    stop_pwr = 10
if harvester_type == "nxp":
    meas_voltage = "2.0"
    start_pwr = -15
    stop_pwr = 15
# ---------------- #

sub_path_name = f"data_{harvester_type}_{cap_size_mf}mf_with_return_loss"
# file_name = "1731707775_868.0_-5.0_3.1"
# sub_path_name = "data_nxp_33mf_with_return_loss"
freq_buffer = [868.0, 920.0]
# freq_buffer = [868.0]

# pwr_buffer = np.linspace(-5, 15, 9) #"-5.0"

level_buffer = np.linspace(start_pwr, stop_pwr, int((stop_pwr-start_pwr)/2.5)+1)

target_voltage = 3.1

change_time_buffer = {'868.0': [],
                      '920.0': []}


for freq in freq_buffer:
    for pwr in level_buffer:

        # Get panda file
        data = get_panda_file(sub_path_name, freq, pwr, meas_voltage)

        # Example: Access individual columns
        timestamps = data["timestamp"]
        voltages = data["voltage_uV"]
        reflected_powers = data["reflected_power_dbm"]

        # Restrict samples
        end_index = 0
        for i, volt in enumerate(voltages):
            if volt > (target_voltage*1e6):
                end_index = i
                break
        timestamps = timestamps[:end_index]
        voltages = voltages[:end_index]
        reflected_powers = reflected_powers[:end_index]

        duration_s = timestamps.iloc[-1]

        # First divmod to get hours, then second divmod to get minutes and remaining seconds
        hours, remainder = divmod(duration_s, 3600)
        minutes, remaining_seconds = divmod(remainder, 60)

        print(f"{hours} hours, {minutes} minutes, {int(remaining_seconds)} seconds")

        change_time_buffer[str(freq)].append(duration_s)


# print(change_time_buffer)
# print(level_buffer)

fig, ax1 = plt.subplots()

for freq in freq_buffer:
    change_time_buffer[str(freq)] = [i/60 for i in change_time_buffer[str(freq)]]
    ax1.plot(level_buffer, change_time_buffer[str(freq)], label=f"{freq} MHz")

# plt.title(f"Charge 33mF capacitor to {target_voltage} V")
ax1.set_xlabel("Input power [dBm]")
ax1.set_ylabel("Charge time [min]")
plt.grid()
plt.legend()
# plt.show()



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

tikzplotlib.save(f"{os.getcwd()}/plot/images/{harvester_type}_charge_time_{target_voltage}_{cap_size_mf}.tex")