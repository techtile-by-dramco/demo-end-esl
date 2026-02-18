# Simple example on how to use the RsInstrument module for remote-controlling yor VISA instrument
# Preconditions:
# - Installed RsInstrument Python module (see the attached RsInstrument_PythonModule folder Readme.txt)
# - Installed VISA e.g. R&S Visa 5.12.x or newer

from RsInstrument.RsInstrument import RsInstrument
import time
import serial
import struct
import numpy as np
import csv
import os
from tqdm import tqdm

from smc100A import *
from controller import *
from csvhandler import *
from scope import Scope

enable_scope_meas = False

ip = "10.128.48.2"
bw = 2e9
center = 920e6
span = 100e3
rbw = 20

if enable_scope_meas:
  scope = Scope(ip, 0)
# scope.setup(bw, center, span, rbw, 1)

resource_string_1 = 'TCPIP::10.128.48.5::INSTR'  # Standard LAN connection (also called VXI-11)
# resource_string_2 = 'TCPIP::192.168.2.101::hislip0'  # Hi-Speed LAN connection - see 1MA208
# resource_string_3 = 'GPIB::20::INSTR'  # GPIB Connection
# resource_string_4 = 'USB::0x0AAD::0x0119::022019943::INSTR'  # USB-TMC (Test and Measurement Class)
# resource_string_5 = 'RSNRP::0x0095::104015::INSTR'  # R&S Powersensor NRP-Z86
instr = RsInstrument(resource_string_1, True, False)

# FREQ = 917e6
# LVL = -20

csv_header = ['timestamp', 'voltage_uV', 'reflected_power_dbm']


ser = serial.Serial('COM12', 115200, timeout=1)  # Change 'COM1' to your serial port

idn = instr.query_str('*IDN?')
print(f"\nHello, I am: '{idn}'")
print(f'RsInstrument driver version: {instr.driver_version}')
print(f'Visa manufacturer: {instr.visa_manufacturer}')
print(f'Instrument full name: {instr.full_instrument_model_name}')
print(f'Instrument installed options: {",".join(instr.instrument_options)}')

instr.write('*RST')
instr.write('*CLS')

# Non-blocking delay
delay_prev_time = time.time()
delay_prev_progress_bar = time.time()
sample_freq = 5

# Buffer voltage target
bv_target_uV = 3.1e6 #2e6 #3.1e6

cable_loss_gen_circ = 0
cable_loss_circ_scope = 1
cable_loss_circ_harv = 0.8

cable_correction_power_to_harvester = cable_loss_gen_circ + cable_loss_circ_harv
cable_correction_reflected_power = cable_loss_circ_harv + cable_loss_circ_scope

start_pwr = -10
stop_pwr = -10#10

level_buffer = np.linspace(start_pwr, stop_pwr, int((stop_pwr-start_pwr)/2.5)+1)
# level_buffer = [15]

print(level_buffer)

init_meas = True

frequency_buffer = [868e6, 920e6]
# frequency_buffer = [868e6]

# Correction due to cable losses
level_buffer = [x + cable_correction_power_to_harvester for x in level_buffer]

def discharge_buffer_now():
    # Set output RF generator SMC100A
    output(instr, 0)

    # Discharge buffer this take at least 30 seconds
    discharge_buffer(ser)

    # Set the delay duration in seconds
    delay_duration = 31

    # Create a progress bar with tqdm
    for _ in tqdm(range(delay_duration), desc="Discharge buffer", unit="sec"):
        time.sleep(1)  # Simulate 1-second increments

# First discharge buffer
time.sleep(3)
discharge_buffer_now()

for freq in frequency_buffer:

  for level in level_buffer:

    # *** START A MEASUREMENT *** #

    initial_time = round(time.time())

    # freq = 920E6

    # level = 15

    # Setup scope
    if enable_scope_meas:
      if init_meas:
        init_meas = False
        scope.setup(bw, freq, span, rbw, 1)
      else:
        scope.change_settings(freq, span, rbw, 1)

    print(f"*** Start measurement with frequency {freq/1E6} MHz and RF level {level} dBm ***")

    # Define filename
    filename = f"data/{initial_time}_{freq/1E6}_{round((level-cable_correction_power_to_harvester), 1)}_{bv_target_uV/1e6}.csv"

    # Change freq and RF lvl RF generator SMC100A
    change_freq_lvl(instr, freq, level)

    # Create new measurement file
    create_file(filename, csv_header)

    # Set output RF generator SMC100A
    output(instr, 1)

    # Initialize the progress bar
    progress_bar = tqdm(total=bv_target_uV/1e6, desc="Charging", unit="V")

    while 1:

      if (time.time()) >= (delay_prev_time + 1/sample_freq):

        # print(f"t = {time.time()-initial_time}")
        # print(f"t2 = {delay_prev_time-initial_time + 1/sample_freq}")
        # print(1/sample_freq)
        # print(round((time.time()-initial_time)*10)/10)
        
        #  Get voltage
        voltageMicro = get_data(ser)

        #  Get reflected power
        if enable_scope_meas:
          rf_reflected_power = scope.calc_channel_power_peaks(1)[0] + cable_correction_reflected_power
        else:
          rf_reflected_power = 0
        
        # Append to csv file
        append_file(filename, [time.time()-initial_time, voltageMicro, rf_reflected_power])

        # Update the progress bar
        if (time.time()) >= (delay_prev_progress_bar + 1):
          progress_bar.n = voltageMicro/1e6  # Set current progress to the sensed voltage
          progress_bar.refresh()  # Refresh the progress bar display
          delay_prev_progress_bar = time.time()

        #  Update delay_prev_time
        delay_prev_time = time.time()

        # Check voltage exceeding target voltage
        if voltageMicro > bv_target_uV:

          # Finalize the progress bar
          progress_bar.close()

          print(f"Buffer voltage ({bv_target_uV/1e6} V) reached!")

          # Calculate elapsed time
          elapsed_time = time.time() - initial_time

          # Convert elapsed time to hours, minutes, and seconds
          hours = int(elapsed_time // 3600)
          minutes = int((elapsed_time % 3600) // 60)
          seconds = int(elapsed_time % 60)

          print(f"Elapsed time: {hours} hours, {minutes} minutes, {seconds} seconds")

          break

    # Set output RF generator SMC100A
    output(instr, 0)

    # Discharge buffer this take at least 30 seconds
    discharge_buffer(ser)

    # Set the delay duration in seconds
    delay_duration = 31

    # Create a progress bar with tqdm
    for _ in tqdm(range(delay_duration), desc="Discharge buffer", unit="sec"):
        time.sleep(1)  # Simulate 1-second increments

    # *** END OF A MEASUREMENT *** #

instr.close()