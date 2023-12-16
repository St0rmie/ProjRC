
#ifndef __CLIENT__
#define __CLIENT__

/**
 * @file client.hpp
 * @brief Header file for a client instance.
 */

#include <netdb.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>

#include "shared/config.hpp"
#include "shared/protocol.hpp"
#include "shared/verifications.hpp"

#define LOGGED_OUT 0

// -----------------------------------
// | Exceptions				 		 |
// -----------------------------------

/**
 * @brief Thrown when the client fails to create a socket.
 */
class SocketException : public std::runtime_error {
   public:
	SocketException()
		: std::runtime_error("[Error] Failed to create socket.") {}
};

/**
 * @brief Thrown when the client fails to resolve the server's hostname.
 */
class ResolveHostnameException : public std::runtime_error {
   public:
	ResolveHostnameException()
		: std::runtime_error("[Error] Failed to resolve hostname.") {}
};

// -----------------------------------
// | Exceptions				 		 |
// -----------------------------------

/**
 * @brief  Class that represents a client instance. Contains all the information
 about open sockets, the logged in user, address of the server, and
 configuration from initial parameters. It is responsible for calling the
 protocol to send and receive messages.
 */
class Client {
	// User information
	uint32_t _user_id = LOGGED_OUT;
	std::string _password = "";

	// Server information and open fds (sockets)
	std::string _hostname = DEFAULT_HOSTNAME;
	std::string _port = DEFAULT_PORT;

	int _udp_socket_fd = -1;
	int _tcp_socket_fd = -1;
	struct addrinfo* _server_udp_addr = NULL;
	struct addrinfo* _server_tcp_addr = NULL;

	// Internal methods
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
	int sendUdpMessageAndAwaitReply(ProtocolMessage& out_message,
	                                ProtocolMessage& in_message);
	int sendTcpMessageAndAwaitReply(ProtocolMessage& out_message,
	                                ProtocolMessage& in_message);
};

#endif