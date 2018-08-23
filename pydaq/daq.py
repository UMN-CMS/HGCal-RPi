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


# loop
blocks = []
# print('start loop')
#rdout.write('RDOUT_DONE', DONE_PI_MAGIC)
#prev_trig_count = rdout.read('TRIG_COUNT')
#trig_count = prev_trig_count
for event in range(1000000):

    # wait trig++
    # print('{0} wait trig++'.format(event))
    # while trig_count != prev_trig_count+1:
    #     trig_count = rdout.read('TRIG_COUNT')
    #     sleep(0.001)

    # wait blk rdy
    print('{0} wait blk rdy'.format(event))
    blk_rdy = rdout.read('BLOCK_READY')
    while not blk_rdy:
        blk_rdy = rdout.read('BLOCK_READY')
        sleep(0.00001)

    # read data
    print('{0} read data'.format(event))
    data = rdout.readBlock('FIFO', blocksize)
    # print([hex(x) for x in data[:3]])
    print([hex(x) for x in data[3:21]])
    print([hex(x) for x in data[-18:]])
    blocks.append(data)

    sleep(0.005)

    # inc done pi
    print('{0} inc done pi'.format(event))
    rdout.write('RDOUT_DONE', DONE_PI_MAGIC)

#for blk_idx, block in enumerate(blocks):
#    if block[0] != 0xf0000:
#        exit(1)
#    if 0xaaaaaaaa in block or 0x55555555 in block:
#        print blk_idx, 'as!'
