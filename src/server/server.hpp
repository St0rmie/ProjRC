#ifndef __SERVER__
#define __SERVER__

#include <netdb.h>

#include "shared/protocol.hpp"
#include "shared/utils.hpp"

#define EXCEPTION_RETRY_MAX 5

class UnrecoverableException : public std::runtime_error {
   public:
	UnrecoverableException()
		: std::runtime_error("[Error] An unrecoverable exception occured.") {}
};

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
	int udp_socket_fd = -1;
	int tcp_socket_fd = -1;
	struct addrinfo* server_udp_addr = NULL;
	struct addrinfo* server_tcp_addr = NULL;
	Server(int argc, char* argv[]);
	~Server();
	void sendUdpMessage(ProtocolMessage& out_message, Address& addr_from);
	void sendTcpMessage();
};

#endif