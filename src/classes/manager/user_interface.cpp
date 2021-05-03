#include "user_interface.h"

user_interface::user_interface() {
	initscr();
	noecho();
	cbreak();
	getmaxyx(stdscr, screen_heigth_, screen_width_);
}

user_interface::~user_interface() { endwin(); }

void user_interface::initWindows() {
	victim_list_window_ = newwin((screen_heigth_ / 2), (screen_width_ * 2 / 3), 0, 0);
	box(victim_list_window_, 0, 0);
	mvwprintw(victim_list_window_, 0, 1, "[VICTIMS]");

	control_window_ = newwin((screen_heigth_ / 2), (screen_width_ * 2 / 3), (screen_heigth_ / 2), 0);
	keypad(control_window_, true);
	box(control_window_, 0, 0);
	mvwprintw(control_window_, 0, 1, "[CONTROLS]");

	bot_list_window_ = newwin(screen_heigth_, (screen_width_ / 3), 0, (screen_width_ * 2 / 3));
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
	const std::string output = "Connecting to the server...";
	move(screen_heigth_ / 2, (screen_width_ - output.size()) / 2);
	printw(output.c_str());
	getch();
}

void user_interface::updateWindow(WINDOW* wind, std::vector<std::string>& items) {
	std::unique_lock<std::mutex> lock(ui_update_m_);
	for (int i = 0; i < screen_heigth_ - 2; ++i) {
		if (i < items.size()) {
			mvwprintw(wind, i + 1, 1, ("* " + items[i]).c_str());
		} else {
			mvwprintw(wind, i + 1, 1, std::string(20, ' ').c_str());
		}
	}
	wrefresh(wind);
}

void user_interface::updateBots(std::vector<std::string>& data) { updateWindow(bot_list_window_, data); }

void user_interface::updateVictims(std::vector<std::string>& data) { updateWindow(victim_list_window_, data); }
