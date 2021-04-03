#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write_at.hpp>
#include <future>
#include <iostream>
#include <istream>
#include <memory>
#include <queue>
#include <sstream>

namespace io = boost::asio;
class session {
	using tcp = io::ip::tcp;
	using err = boost::system::error_code;
	using on_msg_callback = std::function<void(std::string, session*)>;
	using on_err_callback = std::function<void()>;

    public:
	session(tcp::socket&&);

	void start(on_msg_callback&& onMsg);
	void stop();
	void send(std::string const&);

	void read();
	void write();
	void onRead(err, std::size_t);
	void onWrite(err, std::size_t);

    private:
	std::queue<std::string> msg_queue_;

	tcp::socket socket_;
	io::streambuf buffer_;
	err error_code_;

	on_msg_callback on_message_callback_;
	on_err_callback on_error_callback_;

    protected:
};

#endif	// SESSION_H