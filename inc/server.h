#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/bind/bind.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>

#include "session.h"

namespace io = boost::asio;
using tcp = io::ip::tcp;
using err = boost::system::error_code;

class server {
    public:
	server(io::io_context &, std::uint16_t);
	void start();

    private:
	void notifyAll(std::string const &);
	void responseHandler(std::stringstream &, session *);
	void send();

	void accept();
	void onAccept(err error_code);

	io::io_context &io_context;
	tcp::acceptor acceptor;
	std::optional<tcp::socket> socket;
	std::vector<std::shared_ptr<session>> clients;

};

#endif	// SERVER_H