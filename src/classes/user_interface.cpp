#include "user_interface.h"

user_interface::user_interface() {
	initscr();
	noecho();
	cbreak();
	getmaxyx(stdscr, screen_heigth_, screen_width_);
	visible_ = screen_heigth_ - 4;
}

user_interface::~user_interface() { endwin(); }

void user_interface::loadLogo() {
	const std::string output = "Connected to the server! Press [ENTER] to continue...";
	move(screen_heigth_ / 2, (screen_width_ - output.size()) / 2);
	printw(output.c_str());
	getch();
}

void user_interface::start() {
	loadLogo();
	initWindows();

	wrefresh(main_window_);
	wrefresh(help_commands_window_);

	mainWindowSelector();
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

void user_interface::mainWindowSelector() {
	int choice;
	int current = 0;

	while (true) {
		{
			std::unique_lock<std::mutex> lock(ui_update_m_);
			for (int i = 0; i + main_selector_offset_ < bots_data_.size(); ++i) {
				if (i == current) {
					wattron(main_window_, A_REVERSE);
				}
				if (i < visible_) {
					mvwprintw(main_window_, i + 1, 1, bots_data_[i + main_selector_offset_].c_str());
				}
				wattroff(main_window_, A_REVERSE);
			}
			wrefresh(main_window_);
		}

		choice = wgetch(main_window_);

		if (choice == KEY_UP || choice == 'k') {
			--current;
			if (current == -1) {
				++current;
				if (main_selector_offset_ > 0) {
					--main_selector_offset_;
				}
			};
		}
		if (choice == KEY_DOWN || choice == 'j') {
			++current;
			if (current >= visible_ || current >= bots_data_.size()) {
				--current;
				if (main_selector_offset_ + visible_ < bots_data_.size()) {
					++main_selector_offset_;
				}
			}
		}
		if (choice == 'u') {
			get_bots_data_callback_();
			break;
		}
		if (choice == 'r') {
			remove_bot_callback_(current);
			break;
		}
	}
	mainWindowSelector();
}

void user_interface::updateMainWindowData(std::vector<std::string>& params) {
	std::string current_item;
	int parameters_num = stoi(params[0]);
	std::vector<int> max_params_lenghts(parameters_num, 0);
	std::vector<std::string> data_output;

	for (int i = 1; i < params.size(); ++i) {
		for (int j = 0; j < parameters_num; ++j) {
			current_item = params[i + j];

			if (max_params_lenghts[j] < current_item.size()) {
				max_params_lenghts[j] = current_item.size();
			}
			if (j == 0) {
				bot_ip_addresses_.push_back(current_item);
			}
		}
		i += parameters_num - 1;
	}

	int optimal_separator_size =
	    getOptimalSeparatorSize_(parameters_num, std::accumulate(max_params_lenghts.begin(), max_params_lenghts.end(), 0));
	std::string separator = std::string(optimal_separator_size, ' ');

	for (int i = 1 + parameters_num; i < params.size(); ++i) {
		std::vector<std::string> line;

		for (int j = 0; j < parameters_num; ++j) {
			line.push_back(params[i + j] + std::string(max_params_lenghts[j] - params[i + j].size(), ' '));
		}
		data_output.push_back(boost::algorithm::join(line, separator));
		i += parameters_num - 1;
	}
	bots_data_ = std::move(data_output);

	reRenderMainWindowBox();
	updateMainWindowTitles(params, max_params_lenghts, separator);
	// fillWindow_(main_window_, bots_data_);

	wrefresh(main_window_);
}

void user_interface::updateMainWindowTitles(std::vector<std::string>& params, std::vector<int>& max_lengths,
					    std::string& separator) {
	int parameters_num = stoi(params[0]);
	int x_offset = 1;

	for (int i = 1; i < 1 + parameters_num; ++i) {
		mvwprintw(main_window_, 0, x_offset, params[i].c_str());
		x_offset += max_lengths[i - 1] + separator.size();
	}
	wrefresh(main_window_);
	wmove(main_window_, screen_heigth_, screen_width_);
}

void user_interface::fillWindow_(WINDOW* wind, std::vector<std::string>& items) {
	std::unique_lock<std::mutex> lock(ui_update_m_);
	for (int i = 0; i < items.size(); ++i) {
		mvwprintw(wind, i + 1, 1, (items[i]).c_str());
	}
	wrefresh(wind);
}

void user_interface::reRenderMainWindowBox() {
	werase(main_window_);
	box(main_window_, 0, 0);
}

int user_interface::getOptimalSeparatorSize_(int parameters_num, int line_size) {
	return (int)(screen_width_ - line_size) / parameters_num;
}
