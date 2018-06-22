#include <iostream>
#include <uhal/uhal.hpp>
#include <uhal/definitions.hpp>

#define PRBS_START 0xABCDEF21

int main(int argc,char** argv) {
	uhal::ConnectionManager manager("file://etc/connections.xml");
	uhal::HwInterface rdout = manager.getDevice("hgcal.rdout0");
    rdout.getNode("RDOUT_DONE").write(PRBS_START);
    rdout.dispatch();
	return 0;
}
