#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <curses.h>
#include <ncurses.h>

#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class user_interface {
    public:
	user_interface();
	~user_interface();

	void start();

	void updateBots();
	void updateTitles(std::vector<std::string>&, std::vector<int>&, std::string&);

	std::mutex ui_update_m_;
	std::unordered_map<std::string, std::function<void()>> menu_handlers_;
    std::vector<std::string> bots_data_;

    private:
	void loadLogo();
	void initWindows();
	void reRenderMainWindowBox();
	void fillWindow(WINDOW*, std::vector<std::string>&);
    void mainMenuSelector();
	std::vector<std::string> bots_ips_;
	std::vector<std::string> commands_info_ = {"[u] - update the view", "[j]/[k] - select bot", "[r] - remove selected bot"};

	WINDOW* main_window_;
	WINDOW* help_commands_window_;
	int screen_width_;
	int screen_heigth_;
};

#endif	// USER_INTERFACE_H