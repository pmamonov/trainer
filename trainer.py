#!/usr/bin/python
import serial, sys, struct, time

if len(sys.argv)<2:
  print "USAGE: %s PORT [file [time]]"%sys.argv[0]
  sys.exit(1)

sr=serial.Serial(port=sys.argv[1])

if len(sys.argv)>2: 
  f=open(sys.argv[2], 'w')
  doflush=False
else: 
  f=sys.stdout
  doflush=True

if len(sys.argv)>3: 
  t=60*int(sys.argv[3])
  tstart = time.time()
  print >>sys.stderr, "Started writing '%s' @ %s. Will finish @ %s"%(sys.argv[2], time.ctime(tstart), time.ctime(tstart+t))
else: t=0

b=""


while 1:
  w=sr.read(6)
  b+=w
#  print map(ord, b)
#  sys.stdout.flush()
  i=b.find("\r\n")
#  print i
#  sys.stdout.flush()
  if i>=0 and len(b[i:])>=6:
#    print map(ord, b[i+2:i+6])
    v=struct.unpack('<I', b[i+2:i+6])[0]
    n = v>>31
    v &= 0x7fffffff
    b=b[i+6:]
#    print "\r%1d:%10d"%(n, v/2),
    print >>f, "%1d %10d"%(n, v)
    if doflush: f.flush()
  if t and time.time()-tstart>t:
    print >>sys.stderr, "Done"
    sys.exit(0)
