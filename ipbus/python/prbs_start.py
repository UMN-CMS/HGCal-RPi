import uhal

uhal.setLogLevelTo(uhal.LogLevel.ERROR)

manager = uhal.ConnectionManager('file://etc/connections.xml')

rdouts = [manager.getDevice(rdout_id) for rdout_id in manager.getDevices()]

for rdout in rdouts:
    rdout.getNode('RDOUT_DONE').write(0xABCDEF21)
    rdout.dispatch()
