//
// Created by Markiyan Valyavka on 5/2/21.
//

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

    private:
	std::string bool_to_str(bool);
};

#endif	// BOTNET_MSG_PARSER_H