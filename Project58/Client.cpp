#include<iostream>
#include <string>
#include <thread>
#include <atomic>
#include <boost/asio.hpp>

using tcp = boost::asio::ip::tcp;

class Client {

	boost::asio::io_context& io_contex;
	tcp::socket socket;
	std::string userName;
	std::atomic<bool> done{ false };
	

	static constexpr uint16_t max_length = 1024;

	char read_buffer[max_length];

public:

	explicit Client(boost::asio::io_context& _io_contex, const std::string& host, uint16_t port, const std::string& _userName) : io_contex(_io_contex), socket(io_contex), userName(_userName){


		try {

			tcp::resolver resolver(io_contex);
			auto endpoints = resolver.resolve(host, std::to_string(port));
			boost::asio::connect(socket, endpoints);
			std::cout << "CLient is connected to server: " << host << " " << port << std::endl;

		}
		catch (std::exception& ex) {

			std::cerr << ex.what() << '\n';
			done = true;
		}
	}


public:

	void start() {

		if (done) return;

		std::thread read_thread([this]() { doRead(); });

		doWrite();

		read_thread.join();
	}



	void doRead() {


		while (!done) {
			try {

				boost::system::error_code ec;

				std::size_t length = socket.read_some(boost::asio::buffer(read_buffer, max_length), ec);

				if (ec == boost::asio::error::eof) {

					break;

				}
				else if (ec) {

					std::cout << ec.what() << std::endl;
					break;
				}

				std::cout.write(read_buffer, length);
				std::cout << std::flush;

			}
			catch (std::exception& ex) {

				break;
			}
		}

		done = true;
		socket.close();
		io_contex.stop();
	}



	void doWrite() {

		std::string message;
		while (!done) {

			try {

				std::cout << userName <<  '>';
				std::cout << std::flush;


				if (!std::getline(std::cin, message)) {

					break;
				}

				if (message == "exit" || message == "quit") {

					break;
				}

				std::string fullMessage = userName + ':' + message + '\n';

				boost::system::error_code ec;
				boost::asio::write(socket, boost::asio::buffer(fullMessage), ec);

				if (ec) {

					break;
				}

			}
			catch (std::exception& ex) {

				std::cerr << ex.what() << '\n';
				break;
			}


		}


		done = true;
		socket.close();
		io_contex.stop();

	}

};





int main(void) {


	try {


		boost::asio::io_context contex;

		Client client(contex, "127.0.0.1", 12345, "Dmitiy");
		client.start();
	}
	catch (std::exception& ex) {

		std::cerr << ex.what() << std::endl;
	}



	return 0;
}