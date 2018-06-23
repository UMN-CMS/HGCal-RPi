import uhal
from time import sleep

NUM_SIG = 65535 # max syncboard can count to

uhal.setLogLevelTo(uhal.LogLevel.ERROR)

manager = uhal.ConnectionManager('file://etc/connections.xml')

rdouts = [manager.getDevice(rdout_id) for rdout_id in manager.getDevices()]

for i in range(NUM_SIG):
    count0 = []
    for rdout in rdouts:
        count0.append(rdout.getNode('RDOUT_DONE_COUNT').read())
        rdout.dispatch() # make sure read gets done first
        rdout.getNode('RDOUT_DONE').write(0xABCD4321)
        rdout.dispatch()

    count1 = []
    for rdout in rdouts:
        count1.append(rdout.getNode('RDOUT_DONE_COUNT').read())
        rdout.dispatch()

    for idx in range(len(rdouts)):
        if (count1[idx] - 1) != count0[idx]:
            print 'BAD! RDOUT_DONE_COUNT for', rdouts[idx].id(), 'didn't increment by one!'
            exit()

print 'Done. Counter increments correctly'
