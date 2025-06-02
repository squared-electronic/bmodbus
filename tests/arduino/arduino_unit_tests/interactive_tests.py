print("Hello from child.py")
import sys
import serial
print("imported serial")
import os
port = os.environ.get("SERIAL_PORT", "/dev/ttyNONE_SPECIFIED")
ser = serial.Serial(port, 115200, timeout=1)
print("opened serial port", ser.name)
# read 10 seconds of data
data = b""
for i in range(5):
    next = ser.read(1 * 11520)  # 1 seconds max
    if type(next) is bytes:
        data += next
        trials = data.split(b"All tests completed")
        print("trials", len(trials), "found", i)
        if len(trials) > 2:
            data = trials[1] # This is the first completed set of printouts
            break
print("read data", len(data), "bytes")
# write the data to stdout

print(data.decode('utf-8'))
if "" in data.decode('utf-8')
    sys.exit(0)
sys.exit(1)  # Exit with an error code if no data was read or if the data is empty
# "All tests completed" is printed in between the different runs of all the tests, so split it and
#print("done")