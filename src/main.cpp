#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <ostream>
#include <string>

#include "server.h"

#define PORT_NUMBER 2772

namespace io = boost::asio;
int main(int argc, char* argv[]) {
	io::io_context io_context(BOOST_ASIO_CONCURRENCY_HINT_SAFE);
	server tcp_server(io_context, PORT_NUMBER);
	tcp_server.start();
	io_context.run();

	return 0;
}
