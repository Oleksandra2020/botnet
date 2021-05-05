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

	void updateBots(std::vector<std::string>&);
	void updateVictims(std::vector<std::string>&);

	std::mutex ui_update_m_;
	std::unordered_map<std::string, std::function<void()>> menu_handlers_;

    private:
	void loadLogo();
	void initWindows();
	void reRenderVictimsWindow();
	void reRenderBotsWindow();
	void reRenderMenuWindow();
	void updateWindow(WINDOW*, std::vector<std::string>&);
	std::vector<std::string> victims_ips_;
	std::vector<std::string> bots_ips_;

	WINDOW* control_window_;
	WINDOW* bot_list_window_;
	WINDOW* victim_list_window_;

	int screen_width_;
	int screen_heigth_;
};

#endif	// USER_INTERFACE_H