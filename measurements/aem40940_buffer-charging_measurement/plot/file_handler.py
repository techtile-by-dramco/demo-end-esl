import pandas as pd
import os
import matplotlib.pyplot as plt
import numpy as np
import glob

def get_panda_file(sub_path_name, freq, pwr, target_voltage):
  # Path to the CSV file
  csv_file = None

  # Read the CSV file into a DataFrame
  file_pattern = os.path.join(f"{os.getcwd()}/data/{sub_path_name}", f"*_{freq}_{pwr}_{target_voltage}.csv")

  # Use glob to match the file pattern
  files = glob.glob(file_pattern)

  # Check if any files match the pattern
  if files:
      # If files are found, get the first match (if there are multiple, you can choose accordingly)
      csv_file = files[0]
      #print(f"File found: {csv_file}")
  else:
      print(f"No file matching the pattern found: {file_pattern}")
      exit()

  data = None

  # Try reading the file with additional arguments to handle common issues
  try:
      data = pd.read_csv(
          csv_file, 
          header=0,
          names=["timestamp", "voltage_uV", "reflected_power_dbm"], 
          dtype={"timestamp": float, "voltage_uV": float, "reflected_power_dbm": float},
          encoding='utf-8',  # Try different encodings if necessary
          on_bad_lines='skip',  # Skip bad lines instead of error_bad_lines
          low_memory=False  # Process the entire file in one go
      )
      # print(data.head())  # Display the first few rows
  except Exception as e:
      print("Error reading CSV:", e)
      exit()

  return data