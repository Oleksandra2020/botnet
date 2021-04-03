#ifndef SERVER_H
#define SERVER_H

#include <../inc/session.h>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/bind/bind.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>

namespace io = boost::asio;
using tcp = io::ip::tcp;
using err = boost::system::error_code;

using errHandler = std::function<void()>;
using msgHandler = std::function<void(std::string)>;

class server {
    public:
	server(io::io_context &, std::uint16_t);
	void start();
	void send();
	void notifyAll(std::string const &);

	void responseHandler(std::string const &, session *);

	void accept();
	void onAccept(err error_code);

	bool attack_started;
	std::string target;

    private:
	io::io_context &io_context;
	tcp::acceptor acceptor;
	std::optional<tcp::socket> socket;
	std::vector<std::shared_ptr<session>> clients;

    protected:
};

#endif	// SERVER_H