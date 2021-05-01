#include <bits/stdint-uintn.h>

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <ostream>
#include <string>

#include "client.h"
#include "server.h"
#include "thread_pool.h"
#include "victim_manipulation.h"

#define SERVER_INTERNAL_PORT_NUMBER 2772
#define SERVER_EXTERNAL_PORT_NUMBER 3916
#define SERVER_IP ""

namespace io = boost::asio;
int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "[ERROR]: Mode of execution [server/client] is not specified" << std::endl
			  << "Exiting..." << std::endl;
		std::exit(1);
	}

	if (std::string(argv[1]) == "server") {
		std::cout << "[INFO]: Running as server..." << std::endl;

		io::io_context io_context(BOOST_ASIO_CONCURRENCY_HINT_SAFE);
		server tcp_server(io_context, SERVER_INTERNAL_PORT_NUMBER);
		tcp_server.start();
		io_context.run();
	}

	if (std::string(argv[1]) == "client") {
		if (argc < 3) {
			std::cerr << "[ERROR]: LOCAL PORT FOR CLIENT NOT SPECIFIED!" << std::endl << "Exiting..." << std::endl;
			std::exit(2);
		}
		if (SERVER_IP == "") {
			std::cerr << "[ERROR]: EXTERNAL IP FOR SERVER NOT SPECIFIED!" << std::endl << "Exiting..." << std::endl;
			std::exit(3);
		}
		std::cout << "[INFO]: Running as client..." << std::endl;

		io::io_context io_context(BOOST_ASIO_CONCURRENCY_HINT_SAFE);
		client client_tcp(io_context, atoi(argv[2]), SERVER_IP, SERVER_EXTERNAL_PORT_NUMBER);
		client_tcp.start();
		io_context.run();
	}
	return 0;
}