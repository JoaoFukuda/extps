#include "swarm.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
	if (argc != 2) return 1;

	Swarm swarm(argv[1], 64);

	swarm.set_timeout(10);
	swarm.set_retries(3);

	if (swarm.set_ports(65535))
		swarm.run();
}

