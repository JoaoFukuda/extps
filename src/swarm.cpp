#include "swarm.hpp"

#include <sstream>
#include <fstream>
#include <iostream>
#include <mutex>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void open_port(int port, int timeout) {
	auto sock = socket(AF_INET, SOCK_STREAM, 0);
	if (!sock) throw("Could not create socket");

	{
		int opt = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	}

	sockaddr_in hint;
	hint.sin_addr.s_addr = INADDR_ANY;
	hint.sin_port = htons(port);
	hint.sin_family = AF_INET;

	if (bind(sock, reinterpret_cast<sockaddr*>(&hint), sizeof(hint)) != 0) {
		close(sock);
		return;
	}
	if (listen(sock, 1) != 0) {
		close(sock);
		return;
	}

	// set timeout of 5s for connection
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(sock, &rfds);

	timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	int timeout_result = select(sock+1, &rfds, NULL, NULL, &tv);
	if (timeout_result > 0) {
		auto client = accept(sock, NULL, NULL);
		close(client);
	}
	close(sock);
}


bool knock_on(const std::string & address, int port) {
	std::stringstream command;
	command << "/bin/proxychains4 -q nc -z " << address << ' ' << port;
	FILE* connection_output = popen(command.str().c_str(), "r");
	return(pclose(connection_output) == 0);
}

bool test_for(const std::string & address, int port, int timeout) {
	std::thread listener = std::thread(open_port, port, timeout);
	bool result = knock_on(address, port);
	listener.join();
	return result;
}

std::string Swarm::address;
std::vector<int> Swarm::ports;
std::atomic<int> Swarm::current_port;
int Swarm::retries (1);
int Swarm::timeout (5);

Swarm::Swarm(const std::string & address, int max_threads)
	: max_threads(max_threads)
 {
	this->address = address;
 };

Swarm::~Swarm() {
	for (auto & thr : threads) {
		thr.join();
	}
}

bool Swarm::set_ports(const std::string & filename) {
	std::ifstream ports_file(filename);
	if (!ports_file) return false;

	int port;
	while (ports_file >> port) {
		ports.push_back(port);
	}

	ports_file.close();
	return true;
}

bool Swarm::set_ports(int last_port) {
	for (int n = 1; n != last_port; ++n) {
		ports.push_back(n);
	}
	return true;
}

bool Swarm::set_ports(int first_port, int last_port) {
	for (int n = first_port; n != last_port; ++n) {
		ports.push_back(n);
	}
	return true;
}

void Swarm::set_retries(int retries) {
	this->retries = retries;
};

void Swarm::set_timeout(int duration) {
	this->timeout = duration;
};

std::mutex io_mu;

void Swarm::run_tests() {
	while (current_port <= ports.size()) {
		int port = ports[current_port++];
		if (port == 0) continue;
		for (int i = 0; i != retries; ++i) {
			if (test_for(address, port, timeout)) {
				io_mu.lock();
				std::cout << port << std::endl;
				io_mu.unlock();
				break;
			} else if (i == retries - 1) {
				io_mu.lock();
				std::cerr << port << std::endl;
				io_mu.unlock();
			}
		}
	}
}

void Swarm::run() {
	for (int i = 0; i != max_threads; ++i) {
		threads.emplace_back(run_tests);
	}
}
