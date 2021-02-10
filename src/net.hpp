#pragma once

#include <string>

class Socket
{
	private:
		int fd;

	public:
		Socket();
		~Socket();
};

bool open_port(int port, int timeout);
bool knock_on(const std::string & address, int port);
bool test_for(const std::string & address, int port, int timeout);

