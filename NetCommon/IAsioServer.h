#pragma once
#include "net_common.h"
#include "ts_deque.h"
#include "AsioConnection.h"

namespace olc {
	namespace net {
		/// <summary>
		/// Interface of AsioServer
		/// </summary>
		/// <typeparam name="T"></typeparam>
		template <typename T>
		class IAsioServer {
			IAsioServer(uint32_t port) : _acceptor(_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {}
			virtual ~IAsioServer() { Disconnect(); }

		public:
			void Update( size_t nMaxMessage = -1, bool bWait = false ) {
				if (bWait) _dq_message_in.wait();

				size_t message_count = 0;
				while ( message_count < 0 nMaxMessage && !_dq_message_in.empty() ) {
					auto msg = _dq_message_in.pop_front();
					OnMessageArrived(msg.remote, msg.msg );
					message_count++;
				}
			}
			bool Start() {
				try {
					WaitForClientConnection();
					_context_thread = std::thread([this])() { _context.run(); } );
				}
				catch (std::exception& e) {
					std::cerr << "[Server Exception]: " << e.what() << std::endl;
					return false;
				}
				std::cerr << "[Server Started].. " << std::endl;
				return true;
			}
			void Stop() {
				_context.stop();
				if (_context_thread.joinable) _context_thread.join();
				std::cerr << "[Server Stopped].. " << std::endl;
			}
			void WaitForClientConnection() {
				_acceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
					if (!ec) {
<<<<<<< HEAD
						std::cout << "[Server : New Client Connection!" << socket.remote_endpoint()  << std::endl;

						std::shared_ptr<AsioConnection<T>> pNewConnection =
							std::make_shared<AsioConnection<T>>(AsioConnection<T>::owner::server,
								_context, std::move(socket), _dq_message_in);
						if (OnClientConnect(pNewConnection)) {
							_dq_pConnections.push_back(std::move(pNewConnection));
							_dq_pConnections.back()->ConnectToClient(this, _client_id++);
							std::cout << "[Client # " << _dq_pConnections.back()->GetID() << "] : Connection Approved!" << std::endl;
						}
						else {
							std::cout << "[Server] : Connection Denied!" << std::endl;
						}
=======
						std::cout << "[Server : New Client COnnection!" << socket.remote_endpoint()  << std::endl;
						//std::shared_ptr<AsioConnection<T>> new_pConnection =
						//	std::make_shared<AsioConnection<T>>(AsioConnection<T>::owner::server,
						//		_context, std::move(socket), _dq_message_in);
						//if (OnClientConnect(new_pConnection)) {
						//	_dq_pConnections.push_back(std::move(new_pConnection));
						//	_dq_pConnections.back()->ConnectToClient(_client_count++);
						//	std::cout << "[Client # " << _dq_pConnections.back()->GetID() << "] : Connection Approved!" << std::endl;
						//}
						//else {
						//	std::cout << "[Server] : Connection Denied!" << std::endl;
						//}
>>>>>>> d699565fd014e111998127947e65cd12cf1f6811
					}
					// wait for another connection from clients
					WaitForClientConnection();
				});
			}
			void MessageToClient(std::shared_ptr<AsioConnection<T>>& client_pConnection, const message<T>& msg ) {
				if (client_pConnection && client_pConnection->IsConnected()) {
					client_pConnection->Send(msg);
				}
				else {
					OnClientDisconnect(client_pConnection);
					client_pConnection->reset();
					_dq_pConnections.erase(
					std::remove(_dq_pConnections.begin(), _dq_pConnections.end(), client_pConnection), _dq_pConnections.end());
				}
			}
			void MessageToAllClients(const message<T>& msg, std::shared_ptr<AsioConnection<T>>& client_pConnection = nullptr) {
				bool IsAyInvalidClient = false;
				for (auto& client : _dq_pConnections ) {
					if (client && client->IsConnected()) {
						if (client != pIgnoreClient) client->Send(msg);
					}
					else {
						OnClientDisconnect(client);
						client->reset();
						IsAyInvalidClient = true;
					}
				}
				if (IsAyInvalidClient) {
					_dq_pConnections.erase(
						std::remove(_dq_pConnections.begin(), _dq_pConnections.end(), nullptr), _dq_pConnections.end());

				}
			}
		protected:
<<<<<<< HEAD
			virtual bool OnClientConnect(std::shared_ptr<AsioConnection<T>>& pClientConnection) {
				return false;
			}
			virtual void OnClientDisconnect(std::shared_ptr<AsioConnection<T>>& pClientConnection) {
			}

			virtual void OnMessageArrived(std::shared_ptr<AsioConnection<T>>& pClientConnection, const message<T>& msg) {
=======
			virtual bool OnClientConnect(std::shared_ptr<AsioConnection<T>>& pConnection) {
				return false;
			}
			virtual void OnClientDisconnect(std::shared_ptr<AsioConnection<T>>& pConnection) {
			}

			virtual void OnMessageArrived(std::shared_ptr<AsioConnection<T>>& client_pConnection, const message<T>& msg) {
>>>>>>> d699565fd014e111998127947e65cd12cf1f6811

			}
			virtual void OnClientValidate(std::shared_ptr<AsioConnection<T>>& pClientConnection, const message<T>& msg) {

<<<<<<< HEAD
			}

=======
>>>>>>> d699565fd014e111998127947e65cd12cf1f6811
			bool Connect(const std::string& hostname, const std::uint16_t port) {
				try {
					_pConnection = std::make_unique<AsioConnection<T>>();

					//resolve hostname/ip address to tangable physical address
					asio::ip::tcp::resolver resolver(_context);
					_endpoint = resolver.resolve(hostname, std::to_string(port));
					_pConnection->ConnectToServer(_endpoint);
					_context_thread = std::thread([this]() {_context.run(); });
				}
				catch (std::exception& e) {
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
				_pConnection->release();
				//return false;
			}
			bool IsConnected() {
				if (_pConnection) {
					return _pConnection->IsConnected();
				}
				return false;
			}
			ts_deque<owned_message<T>>& InCommingMessage() {
				return _dq_message_in;
			}

		protected:
			asio::io_context _context;
			std::thread _context_thread;

			asio::ip::tcp::socket _socket;
			asio::ip::tcp::endpoint _endpoint;
			asio::ip::tcp::acceptor _acceptor;

			std::unique_ptr <AsioConnection<T>> _pConnection;
			std::deque <std::shared_ptr<AsioConnection<T>>> _dq_pConnections;

			//uint32_t _max_clients = 10000;
			uint32_t _client_id = 10000;
		private:
			ts_deque<owned_message<T>> _dq_message_in;
		};
	}
}