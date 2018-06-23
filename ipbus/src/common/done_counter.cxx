#include <uhal/uhal.hpp>
#include <uhal/definitions.hpp>

#include <vector>
#include <string>
#include <iostream>

#define RDOUT_DONE_MAGIC 0xABCD4321

int main(int argc, char** argv) {
	uhal::ConnectionManager manager("file://etc/connections.xml");
    std::vector<std::string> rdout_ids = manager.getDevices();

    std::vector<uhal::HwInterface> rdouts;
    for(std::vector<std::string>::iterator str = rdout_ids.begin(); str != rdout_ids.end(); str++)
        rdouts.push_back(manager.getDevice(str));

	for(int i = 0; i < 65535; i++) {
        std::vector<uint32_t> count0;
        for(std::vector<uhal::HwInterface>::iterator rdout = rdouts.begin(); rdout != rdouts.end(); rdout++) {
            uhal::ValWord<uint32_t> x = rdout.getNode("RDOUT_DONE_COUNT").read();
            rdout.dispatch();
            count0.push_back(x.value());

            rdout.getNode("RDOUT_DONE").write(RDOUT_DONE_MAGIC);
            rdout.dispatch();
        }

		std::vector<uint32_t> count1;
        for(std::vector<uhal::HwInterface>::iterator rdout = rdouts.begin(); rdout != rdouts.end(); rdout++) {
            uhal::ValWord<uint32_t> x = rdout.getNode("RDOUT_DONE_COUNT").read();
            rdout.dispatch();
            count1.push_back(x.value());
        }

        for(int idx = 0; idx < rdouts.size(); idx++) {
            if(count1[idx]-1 != count0[idx]) {
                std::cout << "bad rdout_done count for " << rdouts[idx].getId() << std::endl;
                return 0;
            }
        }
	}

	return 0;
}
