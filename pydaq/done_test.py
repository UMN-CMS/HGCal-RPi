import uhal
from time import sleep
import sys
uhal.setLogLevelTo(uhal.LogLevel.WARNING)

DONE_PI_MAGIC = 0xabcd4343


class ipb_hw():
    def __init__(self, hw):
        self.hw = hw
    def read(self, reg):
        hdr = hw.getNode(reg).read()
        hw.dispatch()
        while not hdr.valid():
            hdr = hw.getNode(reg).read()
            hw.dispatch()
        return hdr.value()

    def readBlock(self, reg, size):
        hdr = hw.getNode(reg).readBlock(size)
        hw.dispatch()
        while not hdr.valid():
            hdr = hw.getNode(reg).readBlock(size)
            hw.dispatch()
        return hdr.value()

    def write(self, reg, val):
        hdr = hw.getNode(reg).write(val)
        hw.dispatch()
        while not hdr.valid():
            hdr = hw.getNode(reg).write(val)
            hw.dispatch()


# setup ipbus
mgr = uhal.ConnectionManager('file://etc/connection.xml')
hw = mgr.getDevice('hgcal.rdout0')
rdout = ipb_hw(hw)


# inc done pi
MAX=16777216
for i in range(MAX):
    rdout.write('RDOUT_DONE', DONE_PI_MAGIC)
    sys.stdout.write('\r%d of %d (%f)' % (i, MAX, 1.*i / MAX))
    sys.stdout.flush()
print ''