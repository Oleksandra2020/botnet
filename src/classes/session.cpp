#include "session.h"

session::session(tcp::socket&& sock, io::io_service& io_context, size_t id) : socket_(std::move(sock)), io_context_(io_context) {
	id_ = id;
	ip_ = "";
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
		std::string output;
		std::stringstream output_stream;

		output.resize(bytes_transferred);
		output_stream << std::istream(&buffer_).rdbuf();
		output_stream.read(&output[0], bytes_transferred);
		output = output.substr(0, output.size() - 1);
		buffer_.consume(bytes_transferred);

		endpoint_ = socket_.remote_endpoint(error_code);
		if (ip_ == "") {
			std::ostringstream ip_stream;
			ip_stream << endpoint_;
			ip_ = ip_stream.str();
		}

		on_message_callback_(output, this);
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
		stop();
	}
}

void session::stop() {
	// io_context_.post([this]() { socket_.close(error_code_); });
	socket_.close(error_code_);
}
