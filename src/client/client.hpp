#ifndef __CLIENT__
#define __CLIENT__

#include <netdb.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <algorithm>

#include "shared/protocol.hpp"
#include "shared/verifications.hpp"
#include "shared/config.hpp"

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
	uint32_t _user_id = LOGGED_OUT;
	std::string _password = "";

	std::string _hostname = DEFAULT_HOSTNAME;
	std::string _port = DEFAULT_PORT;

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
	void configClient(int argc, char* argv[]);

   public:
	Client(int argc, char* argv[]);
	~Client();
	void login(uint32_t user_id, std::string password);
	void logout();
	bool isLoggedIn();
	uint32_t getLoggedInUser();
	std::string getPassword();
	void sendUdpMessageAndAwaitReply(ProtocolMessage& out_message,
	                                 ProtocolMessage& in_message);
	void sendTcpMessageAndAwaitReply(ProtocolMessage& out_message,
	                                 ProtocolMessage& in_message);
};

void printError(std::string error_description);
void printSuccess(std::string success_description);

#endif