#pragma once
#include "net_common.h"
#include "ts_deque.h"
#include "AsioConnection.h"

namespace olc {
	namespace net {
		template <typename T>
		class IAsioClient {
			IAsioClient() : _socket(_context) {}
			virtual ~IAsioClient() { Disconnect(); }

		public :
			bool Connect(const std::string& hostname, const std::uint16_t port) {
				try {

					//resolve hostname/ip address to tangable physical address
					asio::ip::tcp::resolver resolver(_context);
					asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(hostname, std::to_string(port));

					_pConnection = std::make_unique<AsioConnection<T>>(
						AsioConnection<T>::owner::client;
						_context,
						asio::ip::tc::socket(_context),
						_dq_message_in
					);

					_pConnection->ConnectToServer(endpoints);
					_context_thread = std::thread([this]() {_context.run(); });
				}
				catch (std::exception& e){
					return false;
				}
				return true;
			}
			void Disconnect() {
				if (IsConnected) {
					_pConnection->Disconnect();
				}
				_context.stop();
				if (_context_thread.joinable()) _context_thread.join();
				_pConnection.release();
				//return false;
			}
			bool IsConnected() {
				if (_pConnection) {
					return _pConnection->IsConnected();
				}
				return false;
			}
			ts_deque<owned_message<T>>& in_comming() {
				return _dq_message_in;
			}
			void Send(message<T>& msg) {

			}

		protected :
			asio::io_context _context;
			std::thread _context_thread;
			asio::ip::tcp::socket _socket;
			asio::ip::tcp::endpoint _endpoint;
			std::unique_ptr <connection<T>> _pConnection;
		private :
			ts_deque<owned_message<T>> _dq_message_in;
		};
	}
}