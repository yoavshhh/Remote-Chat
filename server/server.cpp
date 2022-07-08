#include <iostream>

#include <olc_net.h>

enum class CustomMsgTypes : uint32_t {
	
	ServerAccept,
	ServerDeny,
    ClientMessage,
    ServerMessage,
    OtherMessage,
};

class CustomServer : public olc::net::server_interface<CustomMsgTypes>
{
public:
	CustomServer(uint16_t nPort) : olc::net::server_interface<CustomMsgTypes>(nPort)
	{

	}

protected:
	virtual bool OnClientConnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client)
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerAccept;
		client->Send(msg);
		return true;
	}

	// Called when a client appears to have disconnected
	virtual void OnClientDisconnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client)
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";
	}

	// Called when a message arrives
	virtual void OnMessage(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client, olc::net::message<CustomMsgTypes>& msg)
	{
        std::cout << "Recieved: " << client->GetID() << ": " << (int)msg.header.id << std::endl;
		switch (msg.header.id)
		{
		case CustomMsgTypes::ClientMessage:
		{
            uint32_t bodySize;
			std::string msgBody;

            msg >> bodySize;
            msgBody.resize(bodySize);
            msg >> msgBody;

			std::cout << client->GetID() << ": " << msgBody << std::endl;

			// Construct a new message and send it to all clients
			olc::net::message<CustomMsgTypes> msg;
			msg.header.id = CustomMsgTypes::OtherMessage;
			msg << msgBody << bodySize << client->GetID();
			MessageAllClients(msg, client);

		}
		break;
		}
	}
};

int main()
{
	CustomServer server(6767);
	server.Start();

	while (1) {
        std::cout << "starting to update server.\n";
		server.Update(200, true);
	}
	std::cout << "Bye bye!\n";
	system("pause");
	return 0;
}