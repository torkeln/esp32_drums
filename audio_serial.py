import serial,sys, time

ser = serial.Serial('/dev/ttyUSB0', 2000000, timeout=1)  # open serial port
with open('/home/torkel/Arduino/i2s_arduino/kst.txt', 'w') as f:
    while True:
        #sys.stdout.write(str(ser.read(),'ASCII'))
        s = str(ser.readline(),'ASCII')
        #print(s)
        f.write(s)
        f.flush()
        #time.sleep(0.001)
    