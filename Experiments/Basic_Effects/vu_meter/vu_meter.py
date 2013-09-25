#!/usr/bin/python
# -*- coding: utf-8 -*-
 
#
#   www.blinkenlight.net
#
#   Copyright 2011 Udo Klein
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program. If not, see http://www.gnu.org/licenses/
 
 
import alsaaudio
import audioop
import serial
import sys
import math
 
port = "/dev/ttyUSB0"
baudrate = 9600
 
if len(sys.argv) == 3:
    output = serial.Serial(sys.argv[1], sys.argv[2])
else:
    print "# Please specify a port and a baudrate"
    print "# using hard coded defaults " + port + " " + str(baudrate)
    output = serial.Serial(port, baudrate)
     
input = alsaaudio.PCM(alsaaudio.PCM_CAPTURE,alsaaudio.PCM_NONBLOCK)
 
input.setchannels(1)                          # Mono
input.setrate(8000)                           # 8000 Hz
input.setformat(alsaaudio.PCM_FORMAT_S16_LE)  # 16 bit little endian
input.setperiodsize(320)
 
lo  = 2000
hi = 32000
 
log_lo = math.log(lo)
log_hi = math.log(hi)
while True:
    len, data = input.read()
    if len > 0:
        # transform data to logarithmic scale
        vu = (math.log(float(max(audioop.max(data, 2),1)))-log_lo)/(log_hi-log_lo)
        # push it to arduino
        output.write(chr(ord('a')+min(max(int(vu*20),0),19)))
