import time
import csv
import numpy as np
import joulescope
import os

# Path to save the experiment data as a YAML file
current_file_path = os.path.abspath(__file__)
current_dir = os.path.dirname(current_file_path)
parent_path = os.path.dirname(current_dir)
filename = os.path.basename(current_dir)

file_path = f"{current_dir}/{round(time.time())}_measurements.csv"

SAMPLE_RATE = 100
WINDOW = 1 / SAMPLE_RATE
DURATION = 15  # seconds

rows = []
t_start = time.time()*1000

def on_statistics(s):
    """Called from USB thread — keep FAST"""
    if s is None:
        return

    rows.append({
        "timestamp": round(time.time()*1000-t_start),
        "current_A": s["signals"]["current"]["µ"]["value"],
        "voltage_V": s["signals"]["voltage"]["µ"]["value"],
        "power_W": s["signals"]["power"]["µ"]["value"],
        "energy_J": s["signals"]["power"]["∫"]["value"],
    })

with joulescope.scan_require_one(config='auto') as js:
    # Zet statistiek-update op 100 Hz
    js.parameter_set("reduction_frequency", SAMPLE_RATE)

    js.statistics_callback = on_statistics
    t_start = time.time()*1000
    js.start()

    time.sleep(DURATION)

    js.stop()

# Schrijf NA de meting naar CSV
with open(f"{file_path}", "w", newline="") as f:
    writer = csv.DictWriter(f, fieldnames=rows[0].keys())
    writer.writeheader()
    writer.writerows(rows)

print(f"Saved {len(rows)} samples")
