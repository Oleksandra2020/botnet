#ifndef BOTNET_MSG_PARSER_H
#define BOTNET_MSG_PARSER_H

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class msg_parser {
    public:
	std::map<std::string, std::vector<std::string>> parse_msg(std::string);
	std::string genCommand(std::string& command, std::vector<std::string>);

    private:
	std::string bool_to_str(bool);
};

#endif	// BOTNET_MSG_PARSER_H