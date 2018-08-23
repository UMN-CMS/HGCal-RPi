import uhal
from time import sleep
import sys
uhal.setLogLevelTo(uhal.LogLevel.WARNING)

blocksize = 30806 #int(sys.argv[1])

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

def get_event(rdout):
    blk_rdy = rdout.read('BLOCK_READY')
    while not blk_rdy:
        blk_rdy = rdout.read('BLOCK_READY')
        sleep(0.00001)

    # read data
    data = rdout.readBlock('FIFO', blocksize)
    sleep(0.005)

    # inc done pi
    rdout.write('RDOUT_DONE', DONE_PI_MAGIC)

    return data


block1 = get_event(rdout)
block2 = get_event(rdout)
trailer1 = block1[30788:]
trailer2 = block2[30788:]

for offset in range(30788):
    equal = True
    for idx in range(18):
        if trailer1[idx] != block1[idx+offset]:
            equal = False
    if equal:
        print(offset)
