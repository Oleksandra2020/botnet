#include "user_interface.h"

user_interface::user_interface() {
	commands_info_ = {"[h] - update bot list", "[l] - update victims list", "[j]/[k] - select bot",
			  "[r] - remove selected item", "[i] - input new victim ip"};

	initscr();
	noecho();
	cbreak();
	getmaxyx(stdscr, screen_heigth_, screen_width_);
}

user_interface::~user_interface() { endwin(); }

void user_interface::start() {
	renderLoadingScreen();
	initWindows();
	mainWindowMenu();
}

void user_interface::initWindows() {
	main_window_ = newwin((screen_heigth_ - 2), screen_width_, 0, 0);
	keypad(main_window_, true);
	reRenderMainWindowBox();

	secondary_window_ = newwin(2, screen_width_, screen_heigth_ - 2, 0);
	keypad(secondary_window_, true);
	reRenderCommandsHelp();
}

void user_interface::renderLoadingScreen() {
	const std::string output = "Connected to the server! Press any key (except power button) to continue...";
	move(screen_heigth_ / 2, (screen_width_ - output.size()) / 2);
	printw(output.c_str());
	getch();
}

void user_interface::reRenderMainWindowBox() {
	werase(main_window_);
	box(main_window_, 0, 0);
	wrefresh(main_window_);
}

void user_interface::reRenderCommandsHelp() {
	werase(secondary_window_);

	int x_pos = 1;
	int y_pos = 0;
	int separator = 2;
	for (auto& item : commands_info_) {
		if (x_pos + item.size() + separator >= screen_width_) {
			x_pos = 1;
			++y_pos;
		}
		mvwprintw(secondary_window_, y_pos, x_pos, item.c_str());
		x_pos += item.size() + separator;
	}
	wrefresh(secondary_window_);
}

void user_interface::reRenderInputWindow() {
	werase(secondary_window_);
	mvwprintw(secondary_window_, 0, 1, ": ");
	wrefresh(secondary_window_);
}

void user_interface::mainWindowMenu() {
	int choice;
	int current = 0;
	int offset = 0;
	int visible = screen_heigth_ - 4;

	while (true) {
		choice = wgetch(main_window_);

		switch (choice) {
			case KEY_UP:
			case 'k':
				--current;
				if (current == -1) {
					++current;
					if (offset > 0) {
						--offset;
					}
				};
				break;
			case KEY_DOWN:
			case 'j':
				++current;
				if (current >= visible || current >= main_window_menu_options_.size()) {
					--current;
					if (offset + visible < main_window_menu_options_.size()) {
						++offset;
					}
				}
				break;
			case 'h':
				get_bots_data_callback_();
				main_window_m_.lock();
				break;
			case 'l':
				get_victims_data_callback_();
				main_window_m_.lock();
				break;
			case 'r':
				if (active_tab_ == "[GET_BOTS_DATA]") {	 //? Definitely need to make better solution in the future
					remove_bot_callback_(main_menu_options_idicators_[current]);
				} else {
					remove_victim_callback_(main_menu_options_idicators_[current]);
				}
				break;
			case 'i':
				std::string input = getInput();
				add_victim_callback_(input);
				break;
		}

		std::unique_lock<std::mutex> lock(main_window_m_);
		for (int i = 0; i + offset < main_window_menu_options_.size(); ++i) {
			if (i == current) {
				wattron(main_window_, A_REVERSE);
			}
			if (i < visible) {
				mvwprintw(main_window_, i + 1, 1, main_window_menu_options_[i + offset].c_str());
			}
			wattroff(main_window_, A_REVERSE);
		}
		wrefresh(main_window_);
	}
}

std::string user_interface::getInput() {
	reRenderInputWindow();
	int ch;
	int offset = 0;
	std::string input;
	std::string str_char;

	while (ch != '\n') {
		ch = wgetch(secondary_window_);
		str_char = std::string(1, ch);
		input += str_char;
		mvwprintw(secondary_window_, 0, 3 + offset, str_char.c_str());
		offset++;
	}
	reRenderCommandsHelp();
	return input;
}

void user_interface::updateMainWindowMenu(std::vector<std::string>& params) {
	std::string current_item;
	int columns_num = stoi(params[0]);
	std::vector<int> max_params_lenghts(columns_num, 0);
	std::vector<std::string> data_output;
	std::vector<std::string> ip_addresses;

	for (int i = 1; i < params.size(); ++i) {
		for (int j = 0; j < columns_num; ++j) {
			current_item = params[i + j];

			if (max_params_lenghts[j] < current_item.size()) {
				max_params_lenghts[j] = current_item.size();
			}
			if (j == 0 && i > columns_num) {
				ip_addresses.push_back(current_item);
			}
		}
		i += columns_num - 1;
	}

	int optimal_separator_size =
	    getOptimalSeparatorSize_(columns_num, std::accumulate(max_params_lenghts.begin(), max_params_lenghts.end(), 0));
	std::string separator = std::string(optimal_separator_size, ' ');

	for (int i = 1 + columns_num; i < params.size(); ++i) {
		std::vector<std::string> line;

		for (int j = 0; j < columns_num; ++j) {
			line.push_back(params[i + j] + std::string(max_params_lenghts[j] - params[i + j].size(), ' '));
		}
		data_output.push_back(boost::algorithm::join(line, separator));
		i += columns_num - 1;
	}

	main_window_menu_options_ = std::move(data_output);
	main_menu_options_idicators_ = std::move(ip_addresses);

	reRenderMainWindowBox();
	updateMainWindowTitles(params, max_params_lenghts, separator);

	wrefresh(main_window_);

	main_window_m_.unlock();
}

void user_interface::updateMainWindowTitles(std::vector<std::string>& params, std::vector<int>& max_lengths,
					    std::string& separator) {
	int columns_num = stoi(params[0]);
	int x_offset = 1;

	for (int i = 1; i < 1 + columns_num; ++i) {
		mvwprintw(main_window_, 0, x_offset, params[i].c_str());
		x_offset += max_lengths[i - 1] + separator.size();
	}
	wrefresh(main_window_);
	wmove(main_window_, screen_heigth_, screen_width_);
}

int user_interface::getOptimalSeparatorSize_(int columns_num, int line_size) {
	return (int)(screen_width_ - line_size) / columns_num;
}
