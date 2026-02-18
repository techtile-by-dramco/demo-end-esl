import csv

# Create and open the file with a header
def create_file(filename, header):
  with open(filename, mode='w', newline='') as file:
      writer = csv.writer(file)
      writer.writerow(header)  # Write header

# Open file in append mode and add new rows
def append_file(filename, data):
  with open(filename, mode='a', newline='') as file:
      writer = csv.writer(file)
      writer.writerow(data)  # Write the row of integers