import pandas as pd
import os
import matplotlib.pyplot as plt
import numpy as np
import glob

from file_handler import *

# # file_name = "1731707775_868.0_-5.0_3.1"
# sub_path_name = "data_33mf_with_return_loss"

# freq_buffer = [868.0, 920.0]
# pwr_buffer = np.linspace(-5, 15, 9) #"-5.0"
# target_voltage = "3.1"

# reflected_powers_buffer = {'868.0': [],
#                            '920.0': []}

# #change_time_buffer['868'].append("2")

# #print(change_time_buffer['868'])
# #exit()

sub_path_name = "data_nxp_33mf_with_return_loss"

freq_buffer = [868.0, 920.0]
# freq_buffer = [868.0]

# pwr_buffer = np.linspace(-5, 15, 9) #"-5.0"
start_pwr = -15
stop_pwr = 15

level_buffer = np.linspace(start_pwr, stop_pwr, int((stop_pwr-start_pwr)/2.5)+1)
target_voltage = "2.0"#"3.1"

efficiency = {'868.0': [],
              '920.0': []}
input_energy = {'868.0': [],
              '920.0': []}
stored_energy = {'868.0': [],
              '920.0': []}
reflected_powers_buffer = {'868.0': [],
                           '920.0': []}

for freq in freq_buffer:
    for pwr in level_buffer:

        # Get panda file
        data = get_panda_file(sub_path_name, freq, pwr, target_voltage)

        # Example: Access individual columns
        timestamps = data["timestamp"]
        voltages = data["voltage_uV"]
        reflected_powers = data["reflected_power_dbm"]

        reflected_powers_buffer[str(freq)].append(np.mean(reflected_powers))



# Create the figure and primary axis
fig, ax1 = plt.subplots()

# Create the secondary y-axis
ax2 = ax1.twinx()  # Only call twinx() once

# Plot reflected powers on the primary y-axis
for freq in freq_buffer:
    # Plot reflected power
    ax1.plot(level_buffer, reflected_powers_buffer[str(freq)], label=f"Reflected Power {freq}")
    
    # Calculate efficiency
    efficiency = [(10**(pwr/10) / (10**(level_buffer[i]/10))) * 100 for i, pwr in enumerate(reflected_powers_buffer[str(freq)])]
    print(f"Efficiency for {freq}: {efficiency}")
    
    # Plot efficiency on the secondary y-axis
    ax2.plot(level_buffer, efficiency, label=f"Reflected vs input power {freq}", linestyle="--")

# Customize the primary axis
ax1.set_xlabel("Input power [dBm]")
ax1.set_ylabel("Reflected power [dBm]")
ax1.grid()
ax1.legend(loc="upper left")

# Customize the secondary axis
ax2.set_ylabel("Reflected vs input power [%]")
ax2.legend(loc="center left",bbox_to_anchor=(0, 0.75))

# Title
plt.title(f"Charge 33mF capacitor to {target_voltage} V")

# Show plot
plt.show()