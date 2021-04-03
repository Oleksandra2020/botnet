#include <boost/asio/io_context.hpp>
#include <iostream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>

#include "../inc/server.h"

namespace io = boost::asio;
int main(int argc, char* argv[]) {
        // if (argc < 2) {
        //         std::cerr << "FEWER ARGUMENTS THAN EXPECTED" << std::endl;
        //         return -1;
        // }

        // std::string test_argument = argv[1];
        // std::cout << test_argument << std::endl;
        io::io_context io_context;
        server listenForClients(io_context, 2772);
        listenForClients.start();
        io_context.run();

	return 0;
}
