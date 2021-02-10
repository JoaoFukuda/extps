#pragma once

#include <atomic>
#include <string>
#include <thread>
#include <vector>

struct SwarmConfig
{
	bool prettify;
	bool quiet;
	int retries = 3;
	int timeout = 5;
	int max_threads = 8;
	std::string address;
	std::string closed_out;
	std::string opened_out;
	std::vector<int> ports;

	SwarmConfig();

	bool add_ports(const std::string & filename);
	bool add_ports(int first_port, int last_port);
	bool add_port(int port);
};

class Swarm
{
	private:
		static std::atomic<int> current_port;
		static SwarmConfig option;
		std::vector<std::thread> threads;

		static void run_tests();

	public:
		Swarm(const SwarmConfig & option);
		~Swarm();

		void run();
};

