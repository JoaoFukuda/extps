#include "swarm.hpp"

#include <iostream>
#include <string>
#include <sstream>

SwarmConfig scan_config;

void help()
{
	std::cout << "External Port Scanner - Scan for open and closed ports on your router from TOR\n"
		"(the internet world)\n";
	std::cout << "Usage: extps [option]... <external address>\n";
	std::cout << "\nOpens ports on the local machine and listens for a period of time for incomming connection, then\n"
		"try to connect through TOR to simulate an external connection. If the connection was successful, the port\n"
		"is open, but if the timeout was reached, the port is deemed as closed (even though it might be open)\n";
	std::cout << "\nexternal address - your WAN's IP or URL\n";
	std::cout << "option - one of:\n";
	std::cout << "\t-t N\tRun N concurrent scanners. (Default: 8)\n";
	std::cout << "\t-T N\tTimeout after N seconds listening on a port. (Default: 5)\n";
	std::cout << "\t-r F\tRead ports from file F. Ports need to be separated by a new line. (Default: -p)\n";
	std::cout << "\t-R N\tRetry N times one same port (return result after the first success or after N failed attempts). (Default: 3)\n";
	std::cout << "\t-p PORTS\tDefine ports separated by ','. Ports may be a single port or a range defined by the first\n"
		"\t\t\tand last ports (inclusive) separated by a hyphen. (Default: 1-1024)\n";
	std::cout << "\t-oo F\tWrites open ports to file F\n";
	std::cout << "\t-oc F\tWrites closed ports to file F\n";
	std::cout << "\t-P\tPretty print\n";
	std::cout << "\nDon't forget that you need sudo privileges to listen on ports 1 through 1024, so to scan any of that,\n"
		"run this command with sudo\n";
	exit(0);
}

int to_int (const std::string & text)
{
	std::stringstream stream(text);
	int result;
	stream >> result;
	return result;
}

bool parse_args(int argc, char* argv[])
{
	if (argc < 2) help();

	for (int argi = 1; argi != argc; ++argi) {
		std::string arg = argv[argi];
		if (arg == "-t") {
			scan_config.max_threads = to_int(argv[++argi]);
		}
		else if (arg == "-T") {
			scan_config.timeout = to_int(argv[++argi]);
		}
		else if (arg == "-r") {
			scan_config.ports.clear();
			if (!scan_config.add_ports(argv[++argi])) {
				std::cerr << "[error] Could not open file '" << argv[argi] << "' for reading" << std::endl;
				return false;
			}
		}
		else if (arg == "-R") {
			scan_config.retries = to_int(argv[++argi]);
		}
		else if (arg == "-p") {
			scan_config.ports.clear();
			std::stringstream expr(argv[++argi]);
			int port1, port2;
			char op;
			while (expr >> port1) {
				expr >> op;
				if (op == '-'){
					expr >> port2;
					scan_config.add_ports(port1, port2);
					expr >> op;
				}
				else if (op == ',') {
					scan_config.add_port(port1);
				}
				else {
					std::cerr << "[error] Error on port expression near operator '" << op << '\'' << std::endl;
					return false;
				}
			}
		}
		else if (arg == "-oo") {
			scan_config.opened_out = argv[++argi];
		}
		else if (arg == "-oc") {
			scan_config.closed_out = argv[++argi];
		}
		else if (arg == "-P") {
			scan_config.prettify = true;
		}
		else {
			scan_config.address = arg;
		}
	}

	if (scan_config.address.empty()) {
		std::cerr << "[error] No external address was given" << std::endl;
		return false;
	}

	return true;
}

int main(int argc, char* argv[])
{
	if (parse_args(argc, argv)) {
		Swarm swarm(scan_config);
		swarm.run();
	}
}

