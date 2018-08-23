import uhal
from time import sleep
import sys
uhal.setLogLevelTo(uhal.LogLevel.WARNING)


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

for hx in range(8):
    block_count = rdout.read('BLOCK_COUNT{0}'.format(hx))
    print('hx={0} : block_count={1}'.format(hx, block_count))
