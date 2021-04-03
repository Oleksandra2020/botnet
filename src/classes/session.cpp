#include "../../inc/session.h"

session::session(tcp::socket&& sock) : socket_(std::move(sock)) {}

void session::start(on_msg_callback&& handler_func) {
	on_message_callback_ = std::move(handler_func);
	read();
}

void session::read() {
	io::async_read_until(socket_, buffer_, "\n", [self = this](err error_code, std::size_t bytes_transferred) {
		self->onRead(error_code, bytes_transferred);
	});
}

void session::onRead(err error_code, std::size_t bytes_transferred) {
	error_code_ = error_code;
    
	if (!error_code) {
		std::stringstream output;
		output << std::istream(&buffer_).rdbuf();

		buffer_.consume(bytes_transferred);

		on_message_callback_(output.str(), this);
		read();
	} else {
		socket_.close(error_code);
	}
}

void session::send(std::string const& data) {
	bool idle = msg_queue_.empty();
	msg_queue_.push(data);

	if (idle) {
		write();
	}
}

void session::write() {
	std::string msg = msg_queue_.front();
	io::async_write(socket_, io::buffer(msg), [self = this](err errorCode, std::size_t bytes_transferred) {
		self->onWrite(errorCode, bytes_transferred);
	});
}

void session::onWrite(err error_code, std::size_t bytes_transferred) {
	error_code_ = error_code;
	if (!error_code) {
		msg_queue_.pop();

		if (!msg_queue_.empty()) {
			write();
		}
	} else {
		socket_.close(error_code);
	}
}

void session::stop() { socket_.close(error_code_); }
