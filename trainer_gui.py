import sys, serial, struct
from PyQt4 import QtGui, QtCore
#sr=serial.Serial(port=sys.argv[1])
#b=""

class trainerGui(QtGui.QWidget):
  def __init__(self):
    self.sr=serial.Serial(sys.argv[1], timeout=0.025)
    self.b=""

    super(trainerGui, self).__init__()

    self.pb=QtGui.QProgressBar(self)
    self.pb.setGeometry(10,10,400,30)
    self.pb.setValue(0)
#    self.pb.setRange(0,1000)

    self.pb1=QtGui.QProgressBar(self)
    self.pb1.setGeometry(10,50,400,30)
    self.pb1.setValue(0)
#    self.pb1.setRange(0,1000)

    self.tim=QtCore.QBasicTimer()
    self.tim.start(50, self)
    

    self.show()

  def timerEvent(self, e):
    self.b+=self.sr.read(size=1000)
    vs=[-1,-1]
    while True:
      i=self.b.find("\r\n")
      if i<0 or len(self.b[i:])<6: break
      v=struct.unpack('<I', self.b[i+2:i+6])[0]
      n = v>>31
      v &= 0x7fffffff
      v/=2
      vs[n] = v
#      print "%d: %d"%(n,v)
#      sys.stdout.flush()
      self.b=self.b[i+6:]
#    print "up"
    for pb,v in ((self.pb,vs[0]),(self.pb1,vs[1])):
      if v>0: pb.setValue(1e3/v*100)
      else: pb.setValue(0)
    print "\r%10d %10d"%tuple(vs),
    sys.stdout.flush()

app = QtGui.QApplication(sys.argv)
rw=trainerGui()
sys.exit(app.exec_())

#b1val = IntVar(root)
#b1 = ProgressBar(root, variable=b1val, width=100)
#b1.pack()

"""
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
    print "%1d:%10d"%(n, v/2)
    sys.stdout.flush()
"""
