import uhal
from time import sleep

NUM_SIG = 65535 # max syncboard can count to

uhal.setLogLevelTo(uhal.LogLevel.ERROR)

manager = uhal.ConnectionManager('file://etc/connections.xml')
rdout = manager.getDevice('hgcal.rdout0')

for i in range(NUM_SIG):
    count0 = rdout.getNode('RDOUT_DONE_COUNT').read()
    rdout.dispatch()

    rdout.getNode('RDOUT_DONE').write(0xABCD4321)
    rdout.dispatch()

    count1 = rdout.getNode('RDOUT_DONE_COUNT').read()
    rdout.dispatch()

    print 'loop', i, ':', count0, count1

    if (count1 - 1) != count0:
        print "BAD! RDOUT_DONE_COUNT didn't increment by one!"
        exit()

print 'Done. Counter increments correctly'
