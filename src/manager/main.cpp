#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <ostream>
#include <string>

#include "manager.h"
#include "msg_parser.h"

#define SERVER_INTERNAL_PORT_NUMBER 2772
#define SERVER_EXTERNAL_PORT_NUMBER 3916
#define SERVER_IP ""

namespace io = boost::asio;
int main(int argc, char* argv[]) {
	if (argc < 1) {
		std::cerr << "[ERROR]: LOCAL PORT FOR CLIENT NOT SPECIFIED!" << std::endl << "Exiting..." << std::endl;
		std::exit(2);
	}
	if (SERVER_IP == "") {
		std::cerr << "[ERROR]: EXTERNAL IP FOR SERVER NOT SPECIFIED!" << std::endl << "Exiting..." << std::endl;
		std::exit(3);
	}
	io::io_context io_context(BOOST_ASIO_CONCURRENCY_HINT_SAFE);
	manager admin_tcp(io_context, atoi(argv[0]), SERVER_IP, SERVER_EXTERNAL_PORT_NUMBER);
	admin_tcp.start();
	io_context.run();
	return 0;
}