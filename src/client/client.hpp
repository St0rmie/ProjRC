#ifndef __CLIENT__
#define __CLIENT__

#include <netdb.h>
#include <unistd.h>

#include <cstring>
#include <string>

#include "shared/protocol.hpp"

#define LOGGED_OUT -1

// Thrown when there is an error when trying to create a socket
class SocketException : public std::runtime_error {
   public:
	SocketException()
		: std::runtime_error("[Error] Failed to create socket.") {}
};

// Thrown when the hostname couldn't be resolved
class ResolveHostnameException : public std::runtime_error {
   public:
	ResolveHostnameException()
		: std::runtime_error("[Error] Failed to resolve hostname.") {}
};

class Client {
	int _user_id = LOGGED_OUT;

	int _udp_socket_fd = -1;
	int _tcp_socket_fd = -1;
	struct addrinfo* _server_udp_addr = NULL;
	struct addrinfo* _server_tcp_addr = NULL;

	void resolveServerAddress(std::string& hostname, std::string& port);
	void sendUdpMessage(ProtocolMessage& message);
	void waitForUdpMessage(ProtocolMessage& message);
	void openTcpSocket();
	void sendTcpMessage(ProtocolMessage& message);
	void waitForTcpMessage(ProtocolMessage& message);
	void closeTcpSocket();

   public:
	Client(std::string& hostname, std::string& port);
	~Client();
	void login(int user_id);
	void logout();
	int isLoggedIn();
	int getLoggedInUser();
};

#endif