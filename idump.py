#!/usr/bin/python
import sys, serial, struct, time

if len(sys.argv) < 2:
	print """
USAGE:
	%s /dev/ttyACM?
""" % sys.argv[0]
	sys.exit(1)

sr = serial.Serial(sys.argv[1], 38400)

t = time.time()

while 1:
	r = struct.unpack("<BBH", sr.read(4))
	if r[0] == 0xaa:
		t1 = time.time()
		print "%s %6.3f %d %d %d" % tuple([time.strftime("%Y-%m-%d %H:%M:%S"), t1 - t] + list(r))
		t = t1
	else:
		sr.read(1)
