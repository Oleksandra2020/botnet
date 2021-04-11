#include "server.h"

server::server(io::io_context& io_context, std::uint16_t port)
    : io_context(io_context), acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {}

void server::start() { accept(); }

void server::accept() {
	socket.emplace(io_context);
	acceptor.async_accept(*socket, [self = this](err error_code) { self->onAccept(error_code); });
}

void server::onAccept(err error_code) {
	auto client = clients.emplace_back(std::make_shared<session>(std::move(*socket), io_context));

	client->send("#: Connected succesfully\n");
	client->send("#: to quit type 'q'\n");

	notifyAll("New client connected, (overall number of clients= \n");

	// passes server callback function that will be triggered after
    // reading a new recieved message from client
	client->start(boost::bind(&server::responseHandler, this, boost::placeholders::_1, boost::placeholders::_2));

	start();
}

void server::notifyAll(std::string const& msg) {
	std::string output = "[NOTIFY ALL]: " + msg;
	for (auto& client : clients) {
		client->send(msg);
	}
}

void server::responseHandler(std::stringstream& query, session* client) {
	std::cout << "Incoming query: " << query.str() << std::endl;
	// TODO: normilize incoming queries

	// quit response example
	if (query.str()[0] == 'q') {
		client->stop();
		std::cout << "1 client disconected" << std::endl;
	}
}
