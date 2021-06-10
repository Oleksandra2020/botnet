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
	INCORRECT_EXECUTION_MODE = -2,
	MISSING_CONNECTION_ARGUMENTS = -3,
};

void get_help_info();

namespace io = boost::asio;

int main(int argc, char *argv[]) {
	if (argc < 2) {
		ERROR("Mode of execution not specified!");
		return NO_EXECUTION_MODE;
	}

	std::string execution_mode(argv[1]);
	std::string local_port;
	std::string server_ip;
	std::string server_port;

	if (execution_mode == "server") {
		if (argc < 3) {
			ERROR("Local port for server not specified!");
			get_help_info();
			return MISSING_CONNECTION_ARGUMENTS;
		}

	} else if (execution_mode == "client" || execution_mode == "admin") {
		if (argc < 5) {
			ERROR("Incorrect arguments !!");
			get_help_info();
			return MISSING_CONNECTION_ARGUMENTS;
		}

		local_port = argv[2];
		server_ip = argv[3];
		server_port = argv[4];

	} else {
		ERROR("Invalid execution mode!!");
		get_help_info();
		return INCORRECT_EXECUTION_MODE;
	}
	local_port = argv[2];

	// Starting the appliation

	io::io_context io_context(BOOST_ASIO_CONCURRENCY_HINT_SAFE);

	if (execution_mode == "server") {
		PRINT("Running as a server", "...")

		server tcp_server(io_context, atoi(local_port.c_str()));
		tcp_server.start();
		io_context.run();

	} else if (execution_mode == "client") {
		PRINT("Running as a client", "...")

		victims victims(THREAD_NUM, "", local_port.c_str());

		client client_tcp(io_context, atoi(local_port.c_str()), server_ip, atoi(server_port.c_str()), &victims);
		client_tcp.start();
		io_context.run();

	} else if (execution_mode == "admin") {
		PRINT("Running as an admin", "...")

		manager admin_tcp(io_context, atoi(local_port.c_str()), server_ip, atoi(server_port.c_str()));
		admin_tcp.start();
		io_context.run();
	}

	return 0;
}

void get_help_info() {
	std::cout << "\n\nArguments info:" << std::endl;
	std::cout << "* Starting the server:\n"
		     "  server [local port to run on]\n"
		  << std::endl;
	std::cout << "* Starting the client:\n"
		     "  client [local port to run on] [server ip address] [server external port]\n"
		  << std::endl;
	std::cout << "* Starting the admin:\n"
		     "  admin [local port to run on] [server ip address] [server external port]\n"
		  << std::endl;
}