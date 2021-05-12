#include <iostream>
#include "olc_net.h"
#include "IAsioServer.h"

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


class SimpleServer : public olc::net::IAsioServer<MessageTypes> {
	//void SimpleServer(uint32_t port ) { }
	void PingServer() {
		olc::net::message<MessageTypes> msg;
		msg.header.id = MessageTypes::ServerPing;
		std::chrono::system_clock::time_point Now = std::chrono::system_clock::now();
		msg << Now;
		//Send(msg);
	}
	void MessageAll() {
		olc::net::message<MessageTypes> msg;
		msg.header.id = MessageTypes::MessageAll;
		//Send(msg);
	}
};

int main() {
	SimpleServer server(60000);
	server.Start();

	while (true) {
		server.Update(-1,true);
	}
	return 0;
}