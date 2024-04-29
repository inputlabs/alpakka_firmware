import os
import sys
from glob import glob
from serial import Serial

devices = glob('/dev/*usb*') + glob('/dev/*USB*')
if len(devices) == 0:
    print("No USB serial adapters found")
    exit()

for i, device in enumerate(devices):
    print(f'({i+1}) {device}')
n = int(input('Select device to connect: '))
os.system('clear')

serial = Serial(devices[n-1], baudrate=115200)

while True:
    try:
        char = serial.read()
        sys.stdout.buffer.write(char)
        sys.stdout.flush()
    except KeyboardInterrupt:
        break
