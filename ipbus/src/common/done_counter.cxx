#include <iostream>
#include <uhal/uhal.hpp>
#include <uhal/definitions.hpp>

#define RDOUT_DONE_MAGIC 0xABCD4321

int main(int argc,char** argv) {
	uhal::ConnectionManager manager("file://etc/connections.xml");
	uhal::HwInterface rdout = manager.getDevice("hgcal.rdout0");

	for(int i = 0; i < 65535; i++) {

		uhal::ValWord<uint32_t> x = rdout.getNode("RDOUT_DONE_COUNT").read();
		rdout.dispatch();
		uint32_t count0 = x.value();
		
		rdout.getNode("RDOUT_DONE").write(RDOUT_DONE_MAGIC);
		rdout.dispatch();

		uhal::ValWord<uint32_t> x1 = rdout.getNode("RDOUT_DONE_COUNT").read();
		rdout.dispatch();
		uint32_t count1 = x1.value();

		std::cout << "loop " << i << ": " << count0 << " " << count1 << std::endl;
		if(count1 - 1 != count0) {
			std::cout << "bad!" << std::endl;
			return 0;
		}
	}
	return 0;
}
