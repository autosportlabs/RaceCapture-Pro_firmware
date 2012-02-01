#!/usr/bin/env python

import serial
from pprint import pprint

def call(ser, cmd):
    ser.write(cmd)
    return get_resp(ser)

def get_resp(ser):
    ser.readline()
    resp = ser.readline().strip()
    ser.readline()
    
    return resp


ser = serial.Serial(port='/dev/ttyACM0', baudrate=115200, timeout=0.25)

num_analog = 7
num_freq = 3

for a in range(0, num_analog):
    print "==> Analog", a
    data = {}
    for param in ("Label", "SampleRate", "Scaling"):
        resp = call(ser, "getAnalog%s %d\r" % (param, a))
        
        for k, v in [x.split("=") for x in resp.split(";") if x]:
            data[k] = v.replace('"', '')
        
            
    pprint(data)

for f in range(0, num_freq):
    print "==> Timer", f
    data = {}
    for param in ("Label", "SampleRate", "Cfg", "PulsePerRev", "Divider", "Scaling"):
        resp = call(ser, "getTimer%s %d\r" % (param, f))
        
        for k, v in [x.split("=") for x in resp.split(";") if x]:
            data[k] = v.replace('"', '')
        
    del data['result']
    pprint(data)

ser.close()
