#include "user_interface.h"

#include <ncurses.h>

user_interface::user_interface() {
	initscr();
	noecho();
	cbreak();
	getmaxyx(stdscr, screen_heigth_, screen_width_);
}

user_interface::~user_interface() { endwin(); }

void user_interface::initWindows() {
	victim_list_window_ = newwin((screen_heigth_ / 2), (screen_width_ * 2 / 3), 0, 0);
	reRenderVictimsWindow();

	control_window_ = newwin((screen_heigth_ / 2), (screen_width_ * 2 / 3), (screen_heigth_ / 2), 0);
	keypad(control_window_, true);
	reRenderMenuWindow();

	bot_list_window_ = newwin(screen_heigth_, (screen_width_ / 3), 0, (screen_width_ * 2 / 3));
	reRenderBotsWindow();
}

void user_interface::reRenderVictimsWindow() {
	werase(victim_list_window_);
	box(victim_list_window_, 0, 0);
	mvwprintw(victim_list_window_, 0, 1, "[VICTIMS]");
}
void user_interface::reRenderMenuWindow() {
	werase(control_window_);
	box(control_window_, 0, 0);
	mvwprintw(control_window_, 0, 1, "[CONTROLS]");
}
void user_interface::reRenderBotsWindow() {
	werase(bot_list_window_);
	box(bot_list_window_, 0, 0);
	mvwprintw(bot_list_window_, 0, 1, "[BOTS]");
}

void user_interface::start() {
	loadLogo();
	initWindows();

	wrefresh(victim_list_window_);
	wrefresh(bot_list_window_);
	wrefresh(control_window_);

	std::vector<std::string> choices;
	choices.reserve(menu_handlers_.size());
	for (auto kv : menu_handlers_) {
		choices.push_back(kv.first);
	}

	int choice;
	int current = 0;

	while (true) {
		for (int i = 0; i < choices.size(); ++i) {
			if (i == current) {
				wattron(control_window_, A_REVERSE);
			}
			mvwprintw(control_window_, i + 1, 1, choices[i].c_str());
			wattroff(control_window_, A_REVERSE);
		}
		choice = wgetch(control_window_);
		if (choice == KEY_UP || choice == 'k') {
			--current;
			if (current == -1) {
				++current;
			};
		}
		if (choice == KEY_DOWN || choice == 'j') {
			++current;
			if (current >= choices.size()) {
				--current;
			};
		}
		if (choice == 10) {
			menu_handlers_[choices[current]]();
		}
	}
}

void user_interface::loadLogo() {
	const std::string output = "Connected to the server! Press [ENTER] to continue...";
	move(screen_heigth_ / 2, (screen_width_ - output.size()) / 2);
	printw(output.c_str());
	getch();
}

void user_interface::updateWindow(WINDOW* wind, std::vector<std::string>& items) {
	std::unique_lock<std::mutex> lock(ui_update_m_);
	for (int i = 0; i < items.size(); ++i) {
		mvwprintw(wind, i + 1, 1, ("* " + items[i]).c_str());
	}
	wrefresh(wind);
}

void user_interface::updateBots(std::vector<std::string>& data) {
	reRenderBotsWindow();
	updateWindow(bot_list_window_, data);
}

void user_interface::updateVictims(std::vector<std::string>& data) {
	reRenderVictimsWindow();
	updateWindow(victim_list_window_, data);
}
