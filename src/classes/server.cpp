#include "../../inc/server.h"

server::server(io::io_context& io_context, std::uint16_t port)
    : io_context(io_context), acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {}

void server::start() { accept(); }

void server::accept() {
	socket.emplace(io_context);
	acceptor.async_accept(*socket, [self = this](err error_code) { self->onAccept(error_code); });
}

void server::onAccept(err error_code) {
	auto client = clients.emplace_back(std::make_shared<session>(std::move(*socket)));

	client->send("#: Connected succesfully\n");
	client->send("#: to quit type 'q'\n");

	notifyAll("New client connected, (overall number of clients= \n");

	// passes server callback function (responseHandler) that will be triggered
	// after reading a new recieved message from client
	client->start(boost::bind(&server::responseHandler, this, boost::placeholders::_1, boost::placeholders::_2));

	start();
}

void server::notifyAll(std::string const& msg) {
	std::string output = "[NOTIFY ALL]: " + msg;
	for (auto& client : clients) {
		client->send(msg);
	}
}

void server::responseHandler(std::string const& query, session* client) {
	std::cout << "Incoming query: " << query << std::endl;
	// TODO: normilize incoming queries

	// quit response example
	if (query[0] == 'q') {
		client->stop();
		std::cout << "1 client disconected" << std::endl;
	}
}
