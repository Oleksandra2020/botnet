
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <ostream>
#include <string>

#include "server.h"
#include "../inc/thread_pool.h"
#include "../inc/victim_manipulation.h"


#define PORT_NUMBER 2772

namespace io = boost::asio;


int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cout << "Mode of execution [server/client] is not specified" << std::endl
                  << "Exiting..." << std::endl;
        std::exit(1);
    }

    if (std::string(argv[1]) == "server") {

        std::cout << "Running as server" << std::endl;
        io::io_context io_context(BOOST_ASIO_CONCURRENCY_HINT_SAFE);
        server tcp_server(io_context, PORT_NUMBER);
        tcp_server.start();
        io_context.run();

    } else if (std::string(argv[1]) == "client") {

        std::cout << "Running as client" << std::endl;
        victims v;
        v.add_victim("192.168.0.102", "8080", "192.168.0.103", "8080");
        v.add_victim("192.168.0.102", "1024", "192.168.0.104", "1024");
        v.add_victim("192.168.0.102", "1025", "192.168.0.105", "1025");

    }

    return 0;
}