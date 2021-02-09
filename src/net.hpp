#pragma once

#include <string>

void open_port(int port, int timeout);
bool knock_on(const std::string & address, int port);
bool test_for(const std::string & address, int port, int timeout);

