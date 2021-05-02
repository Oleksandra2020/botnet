#include "client.h"

client::client(io::io_context &io_context, std::uint16_t port, std::string server_ip, std::uint16_t server_port, msg_parser& msg_parser)
    : io_context(io_context), acceptor(io_context, tcp::endpoint(tcp::v4(), port)), server_ip_(server_ip), server_port_(server_port){
    this->msg_parser_ = msg_parser;
}

void client::start() {
	tcp::socket socket(io_context);
	socket_ = std::move(socket);

	err error;
	tcp::endpoint endpoint(io::ip::make_address(server_ip_), server_port_);

	socket_->connect(endpoint, error);

	auto server = tmp_vect_for_session_.emplace_back(std::make_shared<session>(std::move(*socket_), io_context, 1));

	server->start(boost::bind(&client::handleResponse, this, boost::placeholders::_1, boost::placeholders::_2));
	server->send("INIT;flk23f9f_f=fsd\n");
	server->read();
}

client::command_code client::hash_command (std::string const& in_command) {
    if (in_command == "[ALIVE]") return eARE_YOU_ALIVE;

    return eCOMMAND_NOT_FOUND;
}


void client::handleResponse(std::string &query, session *client) {
	std::cout << client->endpoint_ << "[SERVER SAYS]: " << query << std::endl;

    std::map<std::string, std::vector<std::string>> parsed_msg = msg_parser_.parse_msg(query);

    if (parsed_msg["is_valid_msg"][0] == "false") {
        return;
    }


    switch (hash_command(parsed_msg["command"][0])) {

        case eARE_YOU_ALIVE:
            client->send("ALIVE;True\n");
            break;


        case eCOMMAND_NOT_FOUND:
            std::cout << "COMMAND NOT FOUND [FROM CLIENT]";
            break;


        default:
            std::cout << "Can't interpret msg [FROM CLIENT]";
            break;

    }

}
