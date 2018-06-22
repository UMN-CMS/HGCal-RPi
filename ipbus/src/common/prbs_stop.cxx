#include <iostream>
#include <uhal/uhal.hpp>
#include <uhal/definitions.hpp>

#define PRBS_STOP 0xABCDEF20

int main(int argc,char** argv) {
	uhal::ConnectionManager manager("file://etc/connections.xml");
	uhal::HwInterface rdout = manager.getDevice("hgcal.rdout0");
    rdout.getNode("RDOUT_DONE").write(PRBS_STOP);
    rdout.dispatch();
	return 0;
}
