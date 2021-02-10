#include "net.hpp"

#include <string>
#include <sstream>
#include <thread>

#include <memory.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

bool open_port(int port, int timeout)
{
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
		return false;
	}
	if (listen(sock, 1) != 0) {
		close(sock);
		return false;
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
	return(timeout_result > 0);
}

bool knock_on(const std::string & address, int port, int timeout)
{
	auto sock = socket(AF_INET, SOCK_STREAM, 0);
	if (!sock) return false;

	sockaddr_in host;
	host.sin_addr.s_addr = inet_addr("127.0.0.1");
	host.sin_port = htons(9050);
	host.sin_family = AF_INET;

	char proxy_buf[9];

	proxy_buf[0] = 4; //Version
	proxy_buf[1] = 1; //Command
	((uint16_t*)proxy_buf)[1] = htons(port); //Port
	((uint32_t*)proxy_buf)[1] = inet_addr(address.c_str()); //Address
	proxy_buf[8] = 0; //User ID

	int status = connect(sock, reinterpret_cast<sockaddr*>(&host), sizeof(host));
	if (status == 0) {
		send(sock, proxy_buf, 9, 0);

		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);

		timeval tv;
		tv.tv_sec = timeout;
		tv.tv_usec = 0;

		int timeout_result = select(sock+1, &rfds, NULL, NULL, &tv);
		if (timeout_result > 0) {
			recv(sock, proxy_buf, 8, 0);
		}
	}
	close(sock);

	return(proxy_buf[1] == 0x5a);
}

bool test_for(const std::string & address, int port, int timeout)
{
	std::thread listener = std::thread(open_port, port, timeout);
	bool result = knock_on(address, port, timeout);
	listener.join();
	return result;
}


