#include <iostream>
#include <uhal/uhal.hpp>
#include <uhal/definitions.hpp>

#include <vector>
#include <string>

#define PRBS_START 0xABCDEF21

int main(int argc,char** argv) {
	uhal::ConnectionManager manager("file://etc/connections.xml");
    std::vector<std::string> rdout_ids = manager.getDevices();

    std::vector<uhal::HwInterface> rdouts;
    for(std::vector<std::string>::iterator str = rdout_ids.begin(); str != rdout_ids.end(); str++)
        rdouts.push_back(manager.getDevice(str));

    for(std::vector<uhal::HwInterface>::iterator rdout = rdouts.begin(); rdout != rdouts.end(); rdout++) {
        rdout.getNode("RDOUT_DONE").write(PRBS_START);
        rdout.dispatch();
    }

	return 0;
}
