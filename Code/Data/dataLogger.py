##### Arduino Data Logger #####
# connects to serial port of Wio Terminal while running getGasData.ino file 
# writes new csv file
# reads gas sensor data for N02, C2H5CH, VOC, CO
# writes gas sensor values to that csv file

import serial
import csv
from datetime import datetime

arduino_port = "/dev/cu.usbmodem141401" #serial port of Wio Terminal 
baud = 9600 # Wio Terminal uses baud rate of 9600
fileName="Perfume1.csv" # name of the CSV file generated

ser = serial.Serial(arduino_port, baud)
print("Connected to Arduino port:" + arduino_port)
file = open(fileName, "w")  # can switch to "a" if you just want to add
print("Created file")
file.write("Sample #, Time, N0, C2H5CH, VOC, CO" + "\n") # adding the headings of each column 

samples = 300 # how many samples to collect- keep in mind gasGetData.ino uses delays of 1000 ms
              # this code gives 300 samples with 1000 ms, so 300 samples at a 1 sample/sec frequency
print_labels = False
line = 0 # start at 0 because our header is 0 (not real data)
while line <= samples:

    # incoming = ser.read(9999)
    # if len(incoming) > 0:
    if print_labels:
        if line==0:
            print("Printing Column Headers")
        else:
            print("Line " + str(line) + ": writing...")
    getData=str(ser.readline())
    data=getData[2:][:-5] # parsing out the characters
    print(data)

    time = datetime.now()
    str_time = time.strftime("%H:%M:%S") # only log time

    file = open(fileName, "a")
    sampNum = str(line)
    file.write(sampNum + "," + str_time + "," + data + "\n") # write data with a newline
    line = line+1

print("Data collection complete!")
#file.close()
