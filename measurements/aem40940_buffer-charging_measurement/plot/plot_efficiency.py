import pandas as pd
import os
import matplotlib.pyplot as plt
import numpy as np
import glob

from file_handler import *

cap_size_mf = 22

# file_name = "1731707775_868.0_-5.0_3.1"
# sub_path_name = "data_33mf_with_return_loss"
sub_path_name = f"data_aem_{cap_size_mf}mf_with_return_loss"

freq_buffer = [868.0, 920.0]
# freq_buffer = [868.0]

# pwr_buffer = np.linspace(-5, 15, 9) #"-5.0"
start_pwr = -7.5
stop_pwr = 10

level_buffer = np.linspace(start_pwr, stop_pwr, int((stop_pwr-start_pwr)/2.5)+1)
target_voltage = "3.1"

efficiency = {'868.0': [],
              '920.0': []}
input_energy = {'868.0': [],
              '920.0': []}
stored_energy = {'868.0': [],
              '920.0': []}

#change_time_buffer['868'].append("2")

#print(change_time_buffer['868'])
#exit()

for freq in freq_buffer:
    for pwr in level_buffer:

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

        print("---")

        print(f"time 1: {t[0]}")

        # print(len(t), len(v))
        print(10**(pwr/10))
        input_pwr_mW = 10**(pwr/10)

        
        print(f"Time: {t[-1]}")

        # print(input_pwr_mW/1e3*t[-1])

        ie = input_pwr_mW/1e3*(t[-1]-t[0])

        se = 0

        for i in range(len(v)-1):
            se = se + (v[i + 1]**2 - v[i]**2)*22e-3/2

        print(f"Input energy: {ie}")
        print(f"Stored energy: {se}")

        efficiency[str(freq)].append(se/ie*100)
        input_energy[str(freq)].append(ie)
        stored_energy[str(freq)].append(se)

        # duration_s = timestamps.iloc[-1]

        # # First divmod to get hours, then second divmod to get minutes and remaining seconds
        # hours, remainder = divmod(duration_s, 3600)
        # minutes, remaining_seconds = divmod(remainder, 60)


        # print(f"{hours} hours, {minutes} minutes, {int(remaining_seconds)} seconds")

        # change_time_buffer[str(freq)].append(duration_s)


print(efficiency)
print(level_buffer)

fig, ax1 = plt.subplots()

# for freq in freq_buffer:
#     # efficiency[str(freq)] = [i/60 for i in efficiency[str(freq)]]
#     ax1.plot(pwr_buffer, efficiency[str(freq)], label=f"Charge efficiency {freq} MHz")

# plt.title("Charge efficiency")

ax2 = ax1.twinx()


for freq in freq_buffer:
    # efficiency[str(freq)] = [i/60 for i in efficiency[str(freq)]]
    # ax2.plot(pwr_buffer, stored_energy[str(freq)], label=f"Stored energy {freq} MHz")

    # ax1.plot(level_buffer, efficiency[str(freq)], label=f"Charge efficiency {freq} MHz")

    ax2.plot(level_buffer, input_energy[str(freq)], label=f"Input energy {freq} MHz")

ax1.set_xlabel("Input power [dBm]")
ax1.set_ylabel("Charge efficiency [%]")
ax1.grid()
ax1.legend(loc="upper left")

ax2.set_ylabel("Total input energy [J]")
ax2.legend(loc="center left")


# plt.grid()
# plt.legend()

plt.show()
exit()

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

tikzplotlib.save(f"{os.getcwd()}/plot/images/charge_efficiency_{cap_size_mf}.tex")