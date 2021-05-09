#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <curses.h>
#include <ncurses.h>

#include <boost/algorithm/string/join.hpp>
#include <functional>
#include <iostream>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class user_interface {
    public:
	user_interface();
	~user_interface();

	void start();

	void updateMainWindowData(std::vector<std::string>&);
	void updateMainWindowTitles(std::vector<std::string>&, std::vector<int>&, std::string&);

	std::mutex ui_update_m_;
	std::vector<std::string> bots_data_;

	std::function<void()> get_bots_data_callback_;
	std::function<void(int)> remove_bot_callback_;

    private:
	void loadLogo();
	void initWindows();
	void mainWindowSelector();
	void reRenderMainWindowBox();

	std::vector<std::string> bot_ip_addresses_;
	std::vector<std::string> commands_info_ = {"[u] - update the view", "[j]/[k] - select bot", "[r] - remove selected bot"};

	WINDOW* main_window_;
	WINDOW* help_commands_window_;

	int screen_width_;
	int screen_heigth_;

	void fillWindow_(WINDOW*, std::vector<std::string>&);
	int getOptimalSeparatorSize_(int, int);
};

#endif	// USER_INTERFACE_H