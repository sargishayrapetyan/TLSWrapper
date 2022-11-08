#include <iostream>
#include "SecureClient.h"
#include "ServerConfig.h"
#include "SecureServer.h"
#include <thread>

using namespace std::chrono_literals;
void run_server() {
	TLS::ServerConfig conf{"/home/luxoft/workspace/TLSWrapper/keys/localhost.crt", "/home/luxoft/workspace/TLSWrapper/keys/localhost.key"};
	TLS::SecureServer server{conf};
	server.acceptClient();
}

int main() {
	//run_server();
	std::thread t(run_server);
	std::this_thread::sleep_for(5000ms);
	std::cout << "server started! " << std::endl;
	
	TLS::ServerConfig conf{"/home/luxoft/workspace/TLSWrapper/keys/localhost.crt", "/home/luxoft/workspace/TLSWrapper/keys/localhost.key"};
	TLS::SecureClient client{conf};
	if(client.connectToServer()) {
		std::cout << "connection1....." << std::endl;
		client.send("bla");
	}

	t.join();
}
