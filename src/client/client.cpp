/**
 * @file client.cpp
 * @brief Implementation of the client class and main function.
 *
 * This file contains the implementation of the client class, which is
 * responsible for establishing a connection with the server, sending and
 * receiving messages, and managing the user's login status. It also includes
 * the main function of the user application.
 */

#include "client.hpp"

#include "commands.hpp"
#include "output.hpp"

/**
 * @brief  Main function of the client
 * @param  argc: number of arguments
 * @param  *argv[]: array of arguments
 * @retval 0 : termination was successful
 */
int main(int argc, char *argv[]) {
	Client client(argc, argv);      // Client class instance
	CommandManager commandManager;  // CommandManager class instance
	registerCommands(commandManager);

	while (!std::cin.eof()) {
		try {
			// Wait for user to input a command on terminal
			commandManager.waitCommand(client);
		} catch (MessageReceiveException &e) {
			printError("Server didn't answer.");
		} catch (MessageSendException &e) {
			printError("Failed to send message.");
		} catch (MessageBuildingException &e) {
			printError("Failed to build message.");
		} catch (InvalidMessageException &e) {
			printError("Invalid message received (Wrong format).");
		} catch (UnexpectedMessageException &e) {
			printError("Invalid message (ERR).");
		} catch (std::exception &e) {
			printError("Unknown error. " + std::string(e.what()));
		}
	}

	std::cout << "[QUIT] EOF. Shutting down." << std::endl;
	return 0;
}

/**
 * @brief  Configures the client properly based on the arguments received by the
 * application.
 * @param  argc: number of arguments received by the client
 * @param  *argv[]: arguments received by the client
 * @retval None
 */
void Client::configClient(int argc, char *argv[]) {
	int opt;

	// Treats all the options received by the client
	while ((opt = getopt(argc, argv, "hn:p:")) != -1) {
		switch (opt) {
			case 'n':
				// Hostname
				this->_hostname = std::string(optarg);
				break;
			case 'p':
				// Port
				this->_port = std::string(optarg);
				break;
			default:
				printError("Config error.");
				exit(EXIT_FAILURE);
		}
	}

	// Check if the port number is correct
	if (verify_port_number(_port) == -1) {
		exit(EXIT_FAILURE);
	};
}

/**
 * @brief Constructor that initializes the client class.
 * @param  argc: number of arguments received by the client
 * @param  *argv[]: arguments received by the client
 * @throws SocketException
 */
Client::Client(int argc, char *argv[]) {
	configClient(argc, argv);
	// Create a UDP socket
	if ((this->_udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		throw SocketException();
	}
	this->resolveServerAddress(_hostname, _port);
}

/**
 * @brief  Destructor that closes the sockets and frees the memory allocated for
 * sockets and adresses.
 */
Client::~Client() {
	if (this->_udp_socket_fd != -1) {
		close(this->_udp_socket_fd);
	}
	if (this->_tcp_socket_fd != -1) {
		close(this->_tcp_socket_fd);
	}
	if (this->_server_udp_addr != NULL) {
		freeaddrinfo(this->_server_udp_addr);
	}
	if (this->_server_tcp_addr != NULL) {
		freeaddrinfo(this->_server_tcp_addr);
	}
}

/**
 * @brief  Resolves the server address based on the server hostname and port
 * received by the client.
 * @param  hostname: server hostname
 * @param  port: server port
 * @throws ResolveHostnameException
 * @retval None
 */
void Client::resolveServerAddress(std::string &hostname, std::string &port) {
	struct addrinfo hints;
	int errcode;

	// Convert string to C string (char*)
	const char *host = hostname.c_str();
	const char *port_str = port.c_str();

	// Get UDP address
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;       // IPv4
	hints.ai_socktype = SOCK_DGRAM;  // UDP socket
	errcode = getaddrinfo(host, port_str, &hints, &this->_server_udp_addr);
	if (errcode != 0) {
		throw ResolveHostnameException();
	}

	// Get TCP address
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;        // IPv4
	hints.ai_socktype = SOCK_STREAM;  // TCP socket
	errcode = getaddrinfo(host, port_str, &hints, &this->_server_tcp_addr);
	if (errcode != 0) {
		throw ResolveHostnameException();
	}
}

/**
 * @brief  Sends a UDP message already filled according to the protocol and
 * waits for the corresponding answer from the server.
 * @param  &out_message: A generalized protocol message to send to the server.
 * @param  &in_message: A generalized protocol message to receive from the
 * server.
 * @retval 0 : success
 * @retval -1 : failure
 */
int Client::sendUdpMessageAndAwaitReply(ProtocolMessage &out_message,
                                        ProtocolMessage &in_message) {
	// The client will try to send the message to the server a few times before
	// giving up.
	int triesLeft = UDP_MAX_TRIES;
	while (triesLeft > 0) {
		--triesLeft;
		try {
			this->sendUdpMessage(out_message);
			this->waitForUdpMessage(in_message);
			// Successfully exchanged messages with the server.
			return 0;
		} catch (ConnectionTimeoutException &e) {
			if (triesLeft == 0) {
				printError("Couldn't send message");
				return -1;
			}
		} catch (InvalidMessageException &e) {
			printError("Invalid Message.");
			return -1;
		} catch (UnexpectedMessageException &e) {
			printError("Unexpected Message.");
			return -1;
		} catch (ERRCodeMessageException &e) {
			printError("ERR code message received.");
			return -1;
		} catch (MessageReceiveException &e) {
			printError("Couldn't receive message.");
			return -1;
		} catch (MessageSendException &e) {
			printError("Couldn't send message.");
			return -1;
		} catch (...) {
			printError("Unexpected error.");
			return -1;
		}
	}
	return -1;
}

/**
 * @brief  Calls protocol function to send a UDP message.
 * @param  &message: message to send to the server
 * @retval None
 */
void Client::sendUdpMessage(ProtocolMessage &message) {
	send_udp_message(message, _udp_socket_fd, _server_udp_addr->ai_addr,
	                 _server_udp_addr->ai_addrlen, false);
}

/**
 * @brief  Calls protocol function to wait for a UDP message.
 * @param  &message: message to receive from the server
 * @retval None
 */
void Client::waitForUdpMessage(ProtocolMessage &message) {
	await_udp_message(message, _udp_socket_fd);
}

/**
 * @brief Sends a TCP message already filled according to the protocol and waits
 * for the corresponding answer from the server.
 * @param  &out_message: message to send to the server
 * @param  &in_message: message to receive from the server
 * @retval 0 : success
 * @retval -1 : failure
 */
int Client::sendTcpMessageAndAwaitReply(ProtocolMessage &out_message,
                                        ProtocolMessage &in_message) {
	try {
		openTcpSocket();
		sendTcpMessage(out_message);
		waitForTcpMessage(in_message);
	} catch (ConnectionTimeoutException &e) {
		printError("Couldn't send message");
		closeTcpSocket();
		return -1;
	} catch (InvalidMessageException &e) {
		printError("Invalid Message.");
		closeTcpSocket();
		return -1;
	} catch (UnexpectedMessageException &e) {
		printError("Unexpected Message.");
		closeTcpSocket();
		return -1;
	} catch (ERRCodeMessageException &e) {
		printError("ERR code message received.");
		closeTcpSocket();
		return -1;
	} catch (MessageReceiveException &e) {
		printError("Couldn't receive message");
		closeTcpSocket();
		return -1;
	} catch (MessageSendException &e) {
		printError("Couldn't send message.");
		return -1;
	} catch (...) {
		printError("Unexpected error.");
		closeTcpSocket();
		return -1;
	}
	// Successfully exchanged messages with the server.
	closeTcpSocket();
	return 0;
}

/**
 * @brief Opens a TCP socket.
 * @throws SocketException
 * @retval None
 */
void Client::openTcpSocket() {
	// Create a TCP socket
	this->_tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_tcp_socket_fd == -1) {
		throw SocketException();
	}

	// Define timeout for read operations
	struct timeval read_timeout;
	read_timeout.tv_sec = TCP_READ_TIMEOUT_SECONDS;
	read_timeout.tv_usec = TCP_READ_TIMEOUT_USECONDS;
	int errcode = setsockopt(this->_tcp_socket_fd, SOL_SOCKET, SO_RCVTIMEO,
	                         &read_timeout, sizeof(read_timeout));
	if (errcode < 0) {
		throw SocketException();
	}

	// Define timeout for write operations
	struct timeval write_timeout;
	write_timeout.tv_sec = TCP_WRITE_TIMEOUT_SECONDS;
	write_timeout.tv_usec = TCP_WRITE_TIMEOUT_USECONDS;
	errcode = setsockopt(this->_tcp_socket_fd, SOL_SOCKET, SO_SNDTIMEO,
	                     &write_timeout, sizeof(write_timeout));
	if (errcode < 0) {
		throw SocketException();
	}
}

/**
 * @brief  Calls the protocol function that sends a TCP message already filled
 * according to the protocol.
 * @param  &message: message to send to the server
 * @throws ConnectionTimeoutException
 * @retval None
 */
void Client::sendTcpMessage(ProtocolMessage &message) {
	// Establish connection with server
	ssize_t n = connect(_tcp_socket_fd, _server_tcp_addr->ai_addr,
	                    _server_tcp_addr->ai_addrlen);
	if (n == -1) {
		throw ConnectionTimeoutException();
	}

	// Send message
	send_tcp_message(message, _tcp_socket_fd, false);
}

/**
 * @brief  Calls the protocol function that waits for a TCP message from the
 * server.
 * @param  &message: message to receive from the server
 * @retval None
 */
void Client::waitForTcpMessage(ProtocolMessage &message) {
	await_tcp_message(message, _tcp_socket_fd);
};

/**
 * @brief  Closes the TCP socket.
 * @throws SocketException
 * @retval None
 */
void Client::closeTcpSocket() {
	if (close(this->_tcp_socket_fd) != 0) {
		if (errno == EBADF) {
			// fd already closed
			return;
		}
		throw SocketException();
	}
};

/**
 * @brief Defines the logged in user and respective password in the client class
 * for posterior use.
 * @param  user_id: ID of the logged in user.
 * @param  password: Password of the logged in user.
 * @retval None
 */
void Client::login(uint32_t user_id, std::string password) {
	this->_user_id = user_id;
	this->_password = password;
}

/**
 * @brief  Removes the ID and password of the user from the client class.
 * @retval None
 */
void Client::logout() {
	this->_user_id = LOGGED_OUT;
	this->_password = "";
}

/**
 * @brief  Checks if the user is logged in (according to local information and
 * not the server).
 * @retval True if a user is logged in, false otherwise.
 */
bool Client::isLoggedIn() {
	return (_user_id != LOGGED_OUT);
}

/**
 * @brief Returns the ID of the logged in user.
 * @retval ID of the logged in user.
 */
uint32_t Client::getLoggedInUser() {
	return _user_id;
}

/**
 * @brief Returns the password of the logged in user.
 * @retval password of the logged in user.
 */
std::string Client::getPassword() {
	return _password;
}
