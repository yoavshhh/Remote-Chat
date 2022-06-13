#include <iostream>

#include <olc_net.h>
#include <windows.h>

#include <limits>


enum class CustomMsgTypes : uint32_t {
	
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};

class CustomClient : public olc::net::client_interface<CustomMsgTypes> {

public:
	void PingServer() {

		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerPing;

		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

		msg << timeNow;
		Send(msg);
	}

	void MessageAll() {
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::MessageAll;
		Send(msg);
	}
};

// const std::string& ReceiveInputString() {

// 	std::string str;
// 	std::cin >> str;
// 	return str.c_str();
// }

CustomClient c;
bool bQuit = false;

void input_main() {

    while (!bQuit) {
        int func; std::cin >> func;
        switch (func)
        {
        case 1:
        {
            bQuit = true;
        }
        break;
        case 2:
        {
            c.PingServer();
        }
        break;
        case 3:
        {
            c.MessageAll();
        }
        break;
        default:
        break;
        }
    }
}

void comms_main() {
    	while (!bQuit) {

		if (c.IsConnected()) {

			if (!c.Incoming().empty()) {

				auto msg = c.Incoming().pop_front().msg;

				switch (msg.header.id)
				{ 
				case CustomMsgTypes::ServerAccept:
				{
					// Server responded to a connect request
					std::cout << "Server Accepted Connection\n";
				}
				break;
				case CustomMsgTypes::ServerPing:
				{
					std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point timeThen;
					msg >> timeThen;
					std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
				}
				break;
				case CustomMsgTypes::ServerMessage:
				{
					// Server has responded to a ping request
					uint32_t clientID;
					msg >> clientID;
					std::string msgBody = "Default Ping";
					// Extract
					std::cout << "[" << clientID << "] " << msgBody << "\n";
				}
				break;
				default:
				break;
				}
			}
		}
		else {
			std::cout << "Server Down\n";
			bQuit = true;
		}
	}
}

int main() {
	std::string server_ip = "54.91.248.95";// "89.139.121.237";
	std::cout << "Trying to Connect to IP: " << server_ip << "\n";
	c.Connect(server_ip, 6767);


	std::thread inputThr(input_main);

    comms_main();

	if (inputThr.joinable()) inputThr.join();
    
    c.Disconnect();
	system("pause");
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    AllocConsole();
    FILE* fp = nullptr;
    freopen_s(&fp, "CONIN$", "r+", stdin);
    freopen_s(&fp, "CONOUT$", "w+", stdout);
    freopen_s(&fp, "CONOUT$", "w+", stderr);
    main();
}

