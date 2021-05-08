#include "user_interface.h"

#include <ncurses.h>

user_interface::user_interface() {
	initscr();
	noecho();
	cbreak();
	getmaxyx(stdscr, screen_heigth_, screen_width_);
}

user_interface::~user_interface() { endwin(); }

void user_interface::loadLogo() {
	const std::string output = "Connected to the server! Press [ENTER] to continue...";
	move(screen_heigth_ / 2, (screen_width_ - output.size()) / 2);
	printw(output.c_str());
	getch();
}

void user_interface::initWindows() {
	main_window_ = newwin((screen_heigth_ - 2), screen_width_, 0, 0);
	box(main_window_, 0, 0);
	keypad(main_window_, true);

	help_commands_window_ = newwin(2, screen_width_, screen_heigth_ - 2, 0);

	int x_pos = 1;
	int y_pos = 0;
	for (auto& item : commands_info_) {
		mvwprintw(help_commands_window_, y_pos, x_pos, item.c_str());
		x_pos += item.size() + 2;
		if (x_pos > screen_width_) {
			x_pos = 1;
			y_pos++;
		}
	}
}

void user_interface::reRenderMainWindowBox() {
	werase(main_window_);
	box(main_window_, 0, 0);
}

void user_interface::start() {
	loadLogo();
	initWindows();

	wrefresh(main_window_);
	wrefresh(help_commands_window_);

	mainMenuSelector();
}

void user_interface::mainMenuSelector() {
	int choice;
	int current = 0;

	while (true) {
		for (int i = 0; i < bots_data_.size(); ++i) {
			if (i == current) {
				wattron(main_window_, A_REVERSE);
			}
			mvwprintw(main_window_, i + 1, 1, bots_data_[i].c_str());
			wattroff(main_window_, A_REVERSE);
		}
		choice = wgetch(main_window_);
		if (choice == KEY_UP || choice == 'k') {
			--current;
			if (current == -1) {
				++current;
			};
		}
		if (choice == KEY_DOWN || choice == 'j') {
			++current;
			if (current >= bots_data_.size()) {
				--current;
			};
		}
		if (choice == 'u') {
			menu_handlers_["u"]();
			break;
		}
	}
	mainMenuSelector();
}
void user_interface::fillWindow(WINDOW* wind, std::vector<std::string>& items) {
	std::unique_lock<std::mutex> lock(ui_update_m_);
	for (int i = 0; i < items.size(); ++i) {
		mvwprintw(wind, i + 1, 1, (items[i]).c_str());
	}
	wrefresh(wind);
}

void user_interface::updateBots() {
	reRenderMainWindowBox();
	fillWindow(main_window_, bots_data_);
	wrefresh(main_window_);
}

void user_interface::updateTitles(std::vector<std::string>& params, std::vector<int>& max_sizes, std::string& separator) {
	int parameters_num = stoi(params[0]);
	int x_spaces = 1;
	for (int i = 1; i < 1 + parameters_num; ++i) {
		mvwprintw(main_window_, 0, x_spaces, params[i].c_str());
		x_spaces += max_sizes[i - 1] + separator.size();
	}
	wrefresh(main_window_);
	wmove(main_window_, screen_heigth_, screen_width_);
}
