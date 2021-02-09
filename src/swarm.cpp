#include "swarm.hpp"
#include "net.hpp"

#include <sstream>
#include <fstream>
#include <iostream>
#include <mutex>

SwarmConfig::SwarmConfig()
{
	add_ports(1, 1024);
}

bool SwarmConfig::add_ports(const std::string & filename)
{
	std::ifstream ports_file(filename);
	if (!ports_file) return false;

	int port;
	while (ports_file >> port) {
		ports.push_back(port);
	}

	ports_file.close();
	return true;
}

bool SwarmConfig::add_ports(int first_port, int last_port)
{
	for (int n = first_port; n != last_port; ++n) {
		ports.push_back(n);
	}
	return true;
}

bool SwarmConfig::add_port(int port)
{
	ports.push_back(port);
	return true;
}

std::atomic<int> Swarm::current_port(0);
SwarmConfig Swarm::option;

Swarm::Swarm(const SwarmConfig & option)
{
	this->option = option;
};

Swarm::~Swarm()
{
	for (auto & thr : threads) {
		thr.join();
	}
}

std::mutex io_mu;

void Swarm::run_tests()
{
	while (current_port <= option.ports.size()) {
		int port = option.ports[current_port++];
		if (port == 0) continue;
		for (int i = 0; i != option.retries; ++i) {
			if (test_for(option.address, port, option.timeout)) {
				io_mu.lock();
				std::cout << (option.prettify ? "\033[32m" : "") << "[success] " << port << (option.prettify ? "\033[m" : "") << std::endl;
				if (!option.opened_out.empty()) {
					std::ofstream outfile(option.opened_out, std::ios_base::app);
					if (outfile) {
						outfile << port << '\n';
						outfile.close();
					}
				}
				io_mu.unlock();
				break;
			} else if (i == option.retries - 1) {
				io_mu.lock();
				std::cout << (option.prettify ? "\033[31m" : "") << "[failure] " << port << (option.prettify ? "\033[m" : "") << std::endl;
				if (!option.closed_out.empty()) {
					std::ofstream outfile(option.closed_out, std::ios_base::app);
					if (outfile) {
						outfile << port << '\n';
						outfile.close();
					}
				}
				io_mu.unlock();
			}
		}
	}
}

void Swarm::run()
{
	for (int i = 0; i != option.max_threads; ++i) {
		threads.emplace_back(run_tests);
	}
}
