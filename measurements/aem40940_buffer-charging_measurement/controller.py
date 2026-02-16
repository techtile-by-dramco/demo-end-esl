import time
import struct

def discharge_buffer(ser):
  ser.write(b'\x01')

def get_data(ser):
  ser.write(b'\x02')

  # Wait a bit for the response to ensure all bytes are received
  time.sleep(0.1)

  # Read 4 bytes from the serial port
  response = ser.read(4)

  # print(response)

  # Check if we received exactly 4 bytes
  if len(response) == 4:
      # Convert the 4 bytes into a 32-bit integer
      # Assuming little-endian byte order (LSB first)
      microVolt = struct.unpack('<I', response)[0]
      # print("Received integer:", microVolt)
      return microVolt
  else:
      # print("Failed to receive 4 bytes")

    return 0