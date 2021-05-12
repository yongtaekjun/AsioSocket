#include <iostream>
#include "olc_net.h"
#include "IAsioClient.h"

enum class MessageTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
	FireBullet,
	MovePlayer
};


class SimpleClient : public olc::net::IAsioClient<MessageTypes> {
	//void SimpleCLient() { ; }
	void PingServer() {
		olc::net::message<MessageTypes> msg;
		msg.header.id = MessageTypes::ServerPing;
		std::chrono::system_clock::time_point Now = std::chrono::system_clock::now();
		msg << Now;
		Send(msg);
	}
	void MessageAll() {
		olc::net::message<MessageTypes> msg;
		msg.header.id = MessageTypes::MessageAll;
		Send(msg);
	}
};

int main() {
	SimpleClient sc;
	sc.Connect("127.0.0.1", 60000);

	bool key[3] = {false, false, false};
	bool old_key[3] = { false, false, false };

	bool bContinued = true;
	while ( bContinued) {
		if (GetForegroundWindow() == GetConsoleWindow()) {
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
		}
		if (key[0] == old_key[0]) sc.PingServer();
		if (key[2] == old_key[2]) bContinued = false;

		for (int i = 0; i < 3; i++) old_key[i] = key[i];

		if (sc.IsConnected() ) {
			if (sc.in_comming().empty() ) {
				auto msg = sc.in_comming().pop_front()._msg;

				switch (msg.header.id) {
				case MessageTypes::ServerAccept: {
					std::cout << "Server Accepted : " << std::endl;
					break;
				}
				case MessageTypes::ServerPing: {
					std::chrono::system_clock::time_point Before = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point After;
					msg >> After;
					std::cout << "Ping : " << std::chrono::duration<double>(After - Before).count() << std::endl;
					break;
				}
				case MessageTypes::ServerMessage: {
					uint32_t client_id;
					msg >> client_id;
					std::cout << "Hello from : [" << client_id << "]" << std::endl;
					break;
				}
				} // of switch
			}
		}
		else {
			std::cout << "Server Downed!" << std::endl;
			bContinued = false;
		}
	}
	return 0;
}