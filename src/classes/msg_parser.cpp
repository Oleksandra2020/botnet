#include "msg_parser.h"

std::map<std::string, std::vector<std::string>> msg_parser::parse_msg(std::string msg) {
	bool is_valid_msg = true;
	std::map<std::string, std::vector<std::string>> msg_tokens_map;
	std::vector<std::string> msg_tokens;
	boost::split(msg_tokens, msg, boost::is_any_of(" "));

	if (msg_tokens[0] != ":msg") {
		is_valid_msg = false;
		msg_tokens_map.insert(std::make_pair("is_valid_msg", std::vector<std::string>{this->bool_to_str(is_valid_msg)}));
		return msg_tokens_map;
	}

	msg_tokens_map.insert(std::make_pair("is_valid_msg", std::vector<std::string>{this->bool_to_str(is_valid_msg)}));
	msg_tokens_map.insert(std::make_pair("command", std::vector<std::string>{msg_tokens[1]}));
	msg_tokens_map.insert(std::make_pair("params", std::vector<std::string>{}));

	for (int i = 2; i < msg_tokens.size(); i++) {
		msg_tokens_map["params"].push_back(msg_tokens[i]);
	}
	return msg_tokens_map;
}

std::string msg_parser::bool_to_str(bool b) { return b ? "true" : "false"; }

std::string msg_parser::genCommand(std::string& command, std::vector<std::string> items) {
	std::string output = ":msg " + command;
	for (auto& v : items) {
		output += " " + v;
	}
	return output + "\n";
}