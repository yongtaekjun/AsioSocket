#pragma once
#include "net_common.h"
#include "net_message.h"
#include "ts_deque.h"

namespace olc {
	namespace net {

		enum class owner {
			server,
			client
		};

		enum class MessageType {
			success
		};
		// Forward declare
		template <typename T>
		class IAsioServer;

		template <typename T>
		class AsioConnection : public std::enable_shared_from_this<AsioConnection<T>> {
		public :
			AsioConnection(owner parent,
				asio::io_context& context,
				asio::ip::tcp::socket socket,
				ts_queue<owned_message<T>>& dq_message_in) :
				_context = context,
				_socket(std::move(socket)),
				_dq_message_in(dq_message_in)
			{
				_owner_type = parent;
				if (_owner_type == owner::server) {
					_n_handshake_out = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
					_n_handshake_check = Scramble(_n_handshake_out);
				}
				else {
					_n_handshake_in = 0;
					_n_handshake_out = 0;
				}
			}
			virtual ~AsioConnection() {}
		public :
			bool ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints) {
				if (_owner_type == owner::client) {
					asio::async_connect(_socket, endpoints,
						[this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
							if (!ec) {

								ReadValidation(server);
								//ReadHeader();
							}

					});
				}
			}
			void ReadValidation(olc::net::IAsioServer<T>* server = nullptr ) {
				asio::async_read(_socket, asio::buffer(&_n_handshake_in, sizeof(uint64_t)),
					[this,server](std::error_code ec, std::size_t length) {
						if (!ec) {
							if (_owner_type == owner::server ) {
								if (_n_handshake_in == _n_handshake_check) {
									std::cout << "[" << id << "] Validation Success!" << std::endl;
									server->OnClientValidated(this->shared_from_this());
									ReadHeader();
								}
							}
							else {
								_n_handshake_out = Scramble(_n_handshake_in);
								WriteValidation();
							}
						}
						else {
							std::cout << "[" << id << "] Validation Failed!" << std::endl;
							_socket.close();
						}
					}
				);

			}
			void WriteValidation() {
				asio::async_write(_socket, asio::buffer(&_n_handshake_out, sizeof(uint64_t) ),
					[this](std::error_code ec, std::size_t length) {
						if (!ec) {
							if (_owner_type == owner::client ) {
								ReadHeader();
							}
							else {
								_dq_message_out.pop_front();
								if (_dq_message_out.empty()) {
									WriteHeader();
								}
							}
						}
						else {
							std::cout << "[" << id << "] WriteHeader Failed!" << std::endl;
							_socket.close();
						}
					}
				);
			}
<<<<<<< HEAD
			void ConnectToClient(olc::net::IAsioServer<T>* pServer,  uint32_t uid = 0) {
=======
			void ConnectToClient(uint32_t uid = 0) {
>>>>>>> d699565fd014e111998127947e65cd12cf1f6811
				if (_owner_type == owner::server) {
					if (_socket.is_open()) {
						_id = uid;
						WriteValidation();
						ReadValidation(server);
						//ReadHeader();
					}
				}
			}
			bool Disconnect() {
				if (IsConnected())
					asio::post(_context, [this]() {_socket.close(); });
			}
			bool IsConnected() const {
				return _socket.is_open();
			}
		public :
			void Send(const message<T>& msg) {
				asio::post(_context, 
					[this, msg]() {
						bool bWritingMessage = !_dq_message_out.empty();
						_dq_message_out.push_back(msg);
						if (!bWritingMessage)	WriteHeader();

				});
			}
		private :
			void ReadHeader() {
				asio::async_read(_socket, asio::buffer(&_temporary_msg_in.header, sizeof(message_header<T>)),
					[this](std::error_code ec, std::size_t length) {
						if (!ec) {
							if (_temporary_msg_in.header.size > 0 ) {
								_temporary_msg_in.body.resize(_temporary_msg_in.header.size);
								ReadyBody();
							}
							else {
								AddToInCommingMessageQueue();
							}
						}
						else {
							std::cout << "[" << id << "] ReadHeader Failed!" << std::endl;
							_socket.close();
						}
					}
				);
			}
			void ReadBody() {
				asio::async_read(_socket, asio::buffer(_temporary_msg_in.body.data(), _temporary_msg_in.header.size),
					[this](std::error_code ec, std::size_t length) {
						if (!ec) {
							AddToInCommingMessageQueue();
						}
						else {
							std::cout << "[" << id << "] ReadBody Failed!" << std::endl;
							_socket.close();
						}
					}
				);

			}
			void WriteHeader() {
				asio::async_write(_socket, asio::buffer(&_dq_message_out.front().header, sizeof(message_header<T>)),
					[this](std::error_code ec, std::size_t length) {
						if (!ec) {
							if (_dq_message_out.front().body.size() > 0) {
								WriteBody();
							}
							else {
								_dq_message_out.pop_front();
								if (_dq_message_out.empty()) {
									WriteHeader();
								}
							}
						}
						else {
							std::cout << "[" << id << "] WriteHeader Failed!" << std::endl;
							_socket.close();
						}
					}
				);

			}
			void WriteBody() {
				asio::async_write(_socket, asio::buffer(_dq_message_out.front().body.data(), _dq_message_out.body.size()),
					[this](std::error_code ec, std::size_t length) {
						if (!ec) {
							_dq_message_out.pop_front();
							if (_dq_message_out.empty()) WriteHeader();
						}
						else {
							std::cout << "[" << id << "] WriteBody Failed!" << std::endl;
							_socket.close();
						}
					}
				);

			}
			void AddToInCommingMessageQueue() {
				if (_owner_type == owner::server) 
					_dq_message_in.push_back( {this->shared_from_this(), _temporary_msg_in } );
				else 
					_dq_message_in.push_back({ nullptr, _temporary_msg_in });
				ReadHeader();
			}
			uint64_t Scramble(uint64_t number) {
				uint64_t out = number ^ 0xDEADBEEFC0DECAFE;
				out = (out & 0xF0F0F0F0F0F0F0F0) >> 4 | (out & 0xF0F0F0F0F0F0F0F0) << 4;
				return out ^ 0XC0DEFACE12345678;
			}

		protected :
			asio::ip::tcp::socket _socket;
			owner _owner_type = owner::server;
			uint32_t _id = 0;

			// This is used as a shared context
			asio::io_context& _context;
			ts_deque<message<T>> _dq_message_out;
			ts_deque<owned_message<T>>& _dq_message_in;

			message<T>> _temporary_msg_in;

			//Handshake Validation
			uint64_t _n_handshake_out = 0;
			uint64_t _n_handshake_in = 0;
			uint64_t _n_handshake_check = 0;
		};
	}
}