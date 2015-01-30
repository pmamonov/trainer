#!/usr/bin/python
import sys, serial, struct

if len(sys.argv) < 2:
	print """
USAGE:
	%s /dev/ttyACM?
""" % sys.argv[0]
	sys.exit(1)

sr = serial.Serial(sys.argv[1], 38400)

while 1:
	r = struct.unpack("<BBH", sr.read(4))
	if r[0] == 0xaa:
		print "%d %d %d" % r
	else:
		sr.read(1)
