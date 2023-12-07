#ifndef __SERVER__
#define __SERVER__

#include <netdb.h>

#include "shared/protocol.hpp"
#include "shared/utils.hpp"

class Address {
   public:
	int socket;
	struct sockaddr_in addr;
	socklen_t size;
};

class Server {
	std::string _hostname = DEFAULT_HOSTNAME;
	std::string _port = DEFAULT_PORT;
	bool verbose = true;

	void resolveServerAddress(std::string& hostname, std::string& port);
	void sendUdpMessage(ProtocolMessage& message);
	void waitForUdpMessage(ProtocolMessage& message);
	void openTcpSocket();
	void sendTcpMessage(ProtocolMessage& message);
	void waitForTcpMessage(ProtocolMessage& message);
	void closeTcpSocket();
	void configClient(int argc, char* argv[]);

   public:
	Server(int argc, char* argv[]);
	~Server();
	void sendUdpMessage(ProtocolMessage& out_message, Address& addr_from);
	void sendTcpMessage();
};

#endif