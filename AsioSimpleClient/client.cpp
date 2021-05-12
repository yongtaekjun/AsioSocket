#include <iostream>

#ifdef __WIN32
#define __WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>


std::vector<char> read_buffer(1 * 1024);

void async_read_data(asio::ip::tcp::socket& socket) {
	socket.async_read_some(asio::buffer(read_buffer.data(), read_buffer.size()),
		[&](std::error_code ec, std::size_t length) {
			if (ec) return;
			std::cout << "Read Data : " << length << " bites\r\n";
			for (std::size_t i = 0; i < length; i++) std::cout << read_buffer[i];
			async_read_data(socket);
		}
	);
}

int main2() {
	asio::error_code ec;

	asio::io_context context;
	asio::io_context::work idelWork(context);

	std::thread thrContext = std::thread([&]() { context.run(); });

	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("127.0.0.1", ec), 3456);

	asio::ip::tcp::socket socket(context);

	socket.connect(endpoint, ec);

	if (!ec) {
		std::cout << "Success in connection ...." << std::endl;
	}
	else {
		std::cout << "Failes in connection ...." << std::endl;
	}

	if (socket.is_open()) {
		async_read_data(socket);
		std::string s_request =
			"GET /index.html HTTP/1.1\r\n"
			"Host: david-varr.co.uk\r\n"
			"Connection: close\r\n\r\n";
		socket.write_some(asio::buffer(s_request.data(), s_request.size()), ec);
		using namespace std::chrono_literals;

		std::this_thread::sleep_for(20000ms);

		context.stop();
		if (thrContext.joinable()) thrContext.join();

	}
	system("pause");
	return 0;
}