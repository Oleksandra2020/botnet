//
// Created by Markiyan Valyavka on 4/17/21.
//

#include <iostream>
#include <boost/asio.hpp>

namespace io = boost::asio;
namespace ip = io::ip;
using tcp = io::ip::tcp;
using error_code = boost::system::error_code;

int run_client()
{
    io::io_context io_context;
    tcp::socket socket(io_context);
    tcp::endpoint endpoint(ip::make_address("178.137.209.146"), 3916);

    error_code error;
    socket.connect(endpoint, error);

    if(!error)
    {
        std::string dt = "alo alo";
        socket.write_some(io::buffer(dt.data(), dt.size()), error);
        //std::cout << socket.is_open() << std::endl;
        std::cout << "The connection has been established!";
    }
    else
    {
        std::cerr << "Something went wrong :(";
    }

    return 0;
}