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
	void updateMainWindowMenu(std::vector<std::string>&);

	std::mutex main_window_m_;
	std::vector<std::string> main_window_menu_options_;

	std::function<void()> get_bots_data_callback_;
	std::function<void()> get_victims_data_callback_;
	std::function<void(std::string&)> remove_bot_callback_;
	std::function<void(std::string&)> remove_victim_callback_;
	std::function<void(std::string&)> add_victim_callback_;

    std::string active_tab_;

    private:
	void renderLoadingScreen();
	void initWindows();
	void mainWindowMenu();

	void updateMainWindowTitles(std::vector<std::string>&, std::vector<int>&, std::string&);

	void reRenderMainWindowBox();
	void reRenderCommandsHelp();
	void reRenderInputWindow();

	std::string getInput();
	int getOptimalSeparatorSize_(int, int);

	std::vector<std::string> main_menu_options_idicators_;

	WINDOW* main_window_;
	WINDOW* secondary_window_;

	int screen_width_;
	int screen_heigth_;

	std::vector<std::string> commands_info_;
};

#endif	// USER_INTERFACE_H