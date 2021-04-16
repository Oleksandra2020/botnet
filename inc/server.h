#ifndef SERVER_H
#define SERVER_H

#define INACTIVITY_TIMEOUT 10 * 1000000	 // 10secs
#include <unistd.h>

#include <algorithm>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/bind/bind.hpp>
#include <chrono>
#include <cstdint>
#include <future>
#include <iostream>
#include <memory>
#include <optional>
#include <unordered_map>

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
	void handleResponse(std::string &, session *);
	void pingClients();
	void send();
	long int generateId();

	void accept();
	void onAccept(err error_code);

	std::future<void> routine_future_;

	io::io_context &io_context;
	tcp::acceptor acceptor;
	std::optional<tcp::socket> socket;

	std::unordered_map<long int, std::shared_ptr<session>> clients_map;
	std::mutex clients_m;
};

#endif	// SERVER_H