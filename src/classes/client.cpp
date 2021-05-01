#include "client.h"

client::client(io::io_context &io_context, std::uint16_t port, std::string server_ip, std::uint16_t server_port)
    : io_context(io_context), acceptor(io_context, tcp::endpoint(tcp::v4(), port)), server_ip_(server_ip), server_port_(server_port) {}

void client::start() {
	tcp::socket socket(io_context);
	socket_ = std::move(socket);

	err error;
	tcp::endpoint endpoint(io::ip::make_address(server_ip_), server_port_);

	socket_->connect(endpoint, error);

	auto server = tmp_vect_for_session_.emplace_back(std::make_shared<session>(std::move(*socket_), io_context, 1));

	server->start(boost::bind(&client::handleResponse, this, boost::placeholders::_1, boost::placeholders::_2));
	server->send("Hello world!\n");
	server->read();
}

void client::handleResponse(std::string &query, session *client) {
	std::cout << client->endpoint_ << " [SERVER SAYS]: " << query;

	if (query == query) {
		client->send("[ALIVE]\n");
	}
}
