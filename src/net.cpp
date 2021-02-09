#include "net.hpp"

#include <string>
#include <sstream>
#include <thread>

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


