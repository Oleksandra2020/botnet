#include "session.h"

#include <string>

session::session(tcp::socket&& sock, io::io_service& io_context, size_t id) : socket_(std::move(sock)), io_context_(io_context) {
	id_ = id;
	ip_ = "";
	disconnected_ = false;
	inactive_timeout_count_ = 1;
	boost::asio::streambuf::mutable_buffers_type bufs = buffer_.prepare(BUFFER_SIZE_RESERVE);
}

session::~session() { stop(); }

void session::start(on_msg_callback&& handler_func) {
	on_message_callback_ = std::move(handler_func);
	read();
}

void session::read() {
	io::async_read_until(socket_, buffer_, '\n', [self = this](err error_code, std::size_t bytes_transferred) {
		self->onRead(error_code, bytes_transferred);
	});
}

void session::onRead(err error_code, std::size_t bytes_transferred) {
	error_code_ = error_code;
	if (!error_code) {
		endpoint_ = socket_.remote_endpoint(error_code);
		if (ip_.empty()) {
			std::ostringstream ip_stream;
			ip_stream << endpoint_;
			ip_ = ip_stream.str();
		}

		std::istream is(&buffer_);
		std::string line;
		while (is) {
			std::getline(is, line, '\n');
			// PRINT("LINE RAW: ", line);

			if (is) {
				on_message_callback_(line, this);
			}
		};

		buffer_.consume(bytes_transferred);

		boost::asio::steady_timer timer(io_context_, std::chrono::steady_clock::now() + std::chrono::seconds(0));
		timer.async_wait(boost::bind(&session::read, this));

	} else {
		// PRINT("READ ERROR OCCURED: ", error_code.message());
		stop();
	}
}

void session::send(std::string const& data) {
	bool idle = msg_queue_.empty();
	msg_queue_.push(data);
	if (idle) {
		boost::asio::steady_timer timer(io_context_, std::chrono::steady_clock::now() + std::chrono::seconds(0));
		timer.async_wait(boost::bind(&session::write, this));
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
			boost::asio::steady_timer timer(io_context_, std::chrono::steady_clock::now() + std::chrono::seconds(0));
			timer.async_wait(boost::bind(&session::write, this));
		}
	} else {
		stop();
	}
}

void session::stop() {
	if (disconnected_) {
		return;
	}
	PRINT("DISCONNECTING [" + ip_ + "] due to:", error_code_.message());
	disconnected_ = true;
	socket_.close(error_code_);
}
