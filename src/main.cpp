#include <iostream>
#include "SecureClient.h"
#include "ServerConfig.h"
#include "SecureServer.h"
#include <thread>

using namespace std::chrono_literals;
void run_server() {
	TLS::ServerConfig conf{"/home/luxoft/workspace/TLSWrapper/keys/localhost.crt", "/home/luxoft/workspace/TLSWrapper/keys/localhost.key"};
	TLS::SecureServer server{conf};
	server.listen_client();
}

int main() {
	//run_server();
	std::thread t(run_server);
	std::this_thread::sleep_for(200ms);
	TLS::ServerConfig conf{"/home/luxoft/workspace/TLSWrapper/keys/localhost.crt", "/home/luxoft/workspace/TLSWrapper/keys/localhost.key"};
	TLS::SecureClient client{conf};
	client.receive();
	client.send("bla");
	
	TLS::SecureClient client1{conf};
	client1.receive();
	client1.send("bla");

	TLS::SecureClient client2{conf};
	client2.receive();
	client2.send("bla");

	TLS::SecureClient client3{conf};
	client3.receive();
	client3.send("bla");

	std::this_thread::sleep_for(2000ms);
	std::cout << "main end\n";
	t.detach();

	return 0;

}
