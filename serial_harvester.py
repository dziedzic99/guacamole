import serial
from datetime import datetime
import io
port = "/dev/ttyUSB0"
file = "./dump.csv"

ser = serial.Serial(port)

while True:
    try:
        line = str(ser.readline().strip())
        line = line[2:-1]
        line = (datetime.now().strftime("%Y,%d,%m,%H,%M,%S,")) + line
        with open(file, 'a') as f:
            f.write(line + "\n")
    except Exception as E:
        pass

