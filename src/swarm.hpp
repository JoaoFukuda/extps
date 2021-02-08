#include <atomic>
#include <string>
#include <thread>
#include <vector>

class Swarm {
	private:
		static int retries;
		static int timeout;
		static std::string address;
		static std::vector<int> ports;
		static std::atomic<int> current_port;
		std::vector<std::thread> threads;
		int max_threads;

		static void run_tests();

	public:
		Swarm(const std::string & address, int max_threads);
		~Swarm();

		bool set_ports(const std::string & filename);
		bool set_ports(int last_port);
		bool set_ports(int first_port, int last_port);

		void set_retries(int retries);
		void set_timeout(int seconds);

		void run();
};

