#ifndef BOTNET_CLIENT_CLIENT_H
#define BOTNET_CLIENT_CLIENT_H

#include <unistd.h>

#include <algorithm>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/bind/bind.hpp>
#include <chrono>
#include <cstdint>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "msg_parser.h"
#include "session.h"
#include "user_interface.h"

namespace io = boost::asio;
using tcp = io::ip::tcp;
using err = boost::system::error_code;

class manager {
    public:
	manager(io::io_context &, std::uint16_t, std::string, std::uint16_t);
	void start();

    private:
	void accept();
	void onAccept(err error_code);

	void handleResponse(std::string &, session *);
	void handleAlive(std::string &, std::vector<std::string> &, session *);
	void handleInit(std::string &, std::vector<std::string> &, session *);
	void handleGetBots(std::string &, std::vector<std::string> &, session *);
	void handleGetIps(std::string &, std::vector<std::string> &, session *);

	void getBots();
	void getIps();

	// Server session
	io::io_context &io_context_;
	tcp::acceptor acceptor_;
	std::optional<tcp::socket> socket_;
	std::string server_ip_;
	std::uint16_t server_port_;
	std::vector<std::shared_ptr<session>> server_session_container_;

	// Communication
	msg_parser msg_parser_;
	std::unordered_map<std::string, std::function<void(std::string &, std::vector<std::string> &, session *)>>
	    command_handlers_;
	std::string passphrase_;

	// Interaction
	user_interface interactive_;
	std::future<void> ui_start_;
};

#endif	// BOTNET_CLIENT_CLIENT_H