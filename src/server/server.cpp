/**
 * @file server.cpp
 * @brief Implementation of the Server class and related functions.
 */
#include "server.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <csignal>

#include "handlers.hpp"
#include "output.hpp"

// -------------------------------------
// | Signals and termination handling. |
// -------------------------------------

// Global variable to check if SIGINT was received.
bool sig_int = false;

/**
 * @brief  Sig Handler function for SIGINT. Sets the global variable sig_int to
 * true.
 * @param  sig:
 * @retval None
 */
void sig_int_handler(int sig) {
	(void) sig;
	sig_int = true;
}

/**
 * @brief  Terminates the server and prints a message to stdout.
 * @param  server: Server instance to be terminated.
 * @param  process: Process to be terminated. Can be UDP_MESSAGE or TCP_MESSAGE.
 * @retval None
 */
void terminate(Server &server, int process) {
	server.~Server();
	std::string process_name = process == UDP_MESSAGE ? "UDP" : "TCP";
	std::cout << "[SIGINT] Shutting Down " << process_name << "." << std::endl;
	exit(EXIT_SUCCESS);
}

/**
 * @brief  Sets up the signal handlers for SIGINT, SIGPIPE and SIGCHLD.
 * @retval None
 */
void Server::setupSignalHandlers() {
	// ignore SIGPIPE and SIGCHLD
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);

	// handle SIGINT
	struct sigaction sa;
	sa.sa_handler = sig_int_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if (sigaction(SIGINT, &sa, NULL) == -1) {
		throw UnrecoverableException("Setting SIGINT signal handler.");
	}
}

// -------------------------------------
// | Server.						   |
// -------------------------------------

/**
 * @brief  Configures the server based on the parameters passed in the command
 * line.
 * @param  argc: Number of arguments passed in the command line.
 * @param  argv: Arguments passed in the command line.
 * @retval None
 */
void Server::configServer(int argc, char *argv[]) {
	int opt;

	while ((opt = getopt(argc, argv, "p:v")) != -1) {
		switch (opt) {
			case 'v':
				_verbose = true;
				break;
			case 'p':
				_port = std::string(optarg);
				break;
			default:
				std::cout << "[ERROR] Config error." << std::endl;
				exit(EXIT_FAILURE);
		}
	}

	if (verify_port_number(_port) == -1) {
		exit(EXIT_FAILURE);
	};

	setupSignalHandlers();
}

/**
 * @brief  Constructor that configures the server and creates the sockets.
 * @param  argc: Number of arguments passed in the command line.
 * @param  argv: Arguments passed in the command line.
 */
Server::Server(int argc, char *argv[]) {
	configServer(argc, argv);
	// Create a UDP socket
	if ((_udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		throw UnrecoverableException("[ERROR] Couldn't open socket");
	}
	// Creates base for database
	_database.CreateBaseDir(stoi(_port));

	// Setup sockets
	setup_sockets();

	// Resolve server address
	resolveServerAddress(_port);
}

/**
 * @brief  Destructor that closes the sockets and frees the memory allocated for
 * sockets and adresses.
 */
Server::~Server() {
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
 * @brief  Setup the sockets on the server (UDP and TCP).
 * @retval None
 */
void Server::setup_sockets() {
	// Create a UDP socket
	if ((_udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		throw UnrecoverableException("[ERROR] Failed to create a UDP socket");
	}
	// Force reuse of the socket.
	const int enable_udp = 1;
	if (setsockopt(_udp_socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable_udp,
	               sizeof(int)) < 0) {
		throw UnrecoverableException(
			"[ERROR] Failed to set UDP reuse address socket option");
	}

	// Create a TCP socket
	if ((_tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		throw UnrecoverableException("[ERROR] Failed to create a TCP socket");
	}
	const int enable_tcp = 1;

	// Force reuse of the socket.
	if (setsockopt(_tcp_socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable_tcp,
	               sizeof(int)) < 0) {
		throw UnrecoverableException(
			"[ERROR] Failed to set TCP reuse address socket option");
	}
}

/**
 * @brief  Resolves the server address based on the port passed in the command
 * line or default.
 * @param  port: Port
 * @retval None
 */
void Server::resolveServerAddress(std::string &port) {
	struct addrinfo hints;
	int addr_res;
	const char *port_str = port.c_str();
	// Get UDP address
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;       // IPv4
	hints.ai_socktype = SOCK_DGRAM;  // UDP socket
	hints.ai_flags = AI_PASSIVE;     // Listen on 0.0.0.0
	if ((addr_res = getaddrinfo(NULL, port_str, &hints, &_server_udp_addr)) !=
	    0) {
		throw UnrecoverableException(
			std::string("[ERROR] Failed to get address for UDP connection: ") +
			gai_strerror(addr_res));
	}
	// bind socket
	if (bind(_udp_socket_fd, _server_udp_addr->ai_addr,
	         _server_udp_addr->ai_addrlen) < 0) {
		throw UnrecoverableException("[ERROR] Failed to bind UDP address.");
	}

	// Get TCP address
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;        // IPv4
	hints.ai_socktype = SOCK_STREAM;  // TCP socket
	hints.ai_flags = AI_PASSIVE;      // Listen on 0.0.0.0
	if ((addr_res =
	         getaddrinfo(NULL, port.c_str(), &hints, &_server_tcp_addr)) != 0) {
		throw UnrecoverableException(
			std::string("[ERROR] Failed to get address for TCP connection: ") +
			gai_strerror(addr_res));
	}

	if (bind(_tcp_socket_fd, _server_tcp_addr->ai_addr,
	         _server_tcp_addr->ai_addrlen) < 0) {
		throw UnrecoverableException("[ERROR] Failed to bind TCP address");
	}

	std::cout << "Listening for connections on port " << port << std::endl;
}

// -------------------------------------
// | Request Handler and Manager	   |
// -------------------------------------

/**
 * @brief  Registers a request handler in the request manager.
 * @param  handler: Request handler to be registered.
 * @param  type: Type of the request handler. Can be UDP_MESSAGE or TCP_MESSAGE.
 * @retval None
 */
void RequestManager::registerRequest(std::shared_ptr<RequestHandler> handler,
                                     int type) {
	if (type == UDP_MESSAGE) {
		_udp_handlers.insert({handler->_protocol_code, handler});
	} else {
		_tcp_handlers.insert({handler->_protocol_code, handler});
	}
}

/**
 * @brief  Registers all the request handlers in the request manager for each
 * command possible.
 * @retval None
 */
void RequestManager::registerRequestHandlers() {
	registerRequest(std::make_shared<LoginRequest>(), UDP_MESSAGE);
	registerRequest(std::make_shared<LogoutRequest>(), UDP_MESSAGE);
	registerRequest(std::make_shared<UnregisterRequest>(), UDP_MESSAGE);
	registerRequest(std::make_shared<ListAllAuctionsRequest>(), UDP_MESSAGE);
	registerRequest(std::make_shared<ListBiddedAuctionsRequest>(), UDP_MESSAGE);
	registerRequest(std::make_shared<ListStartedAuctionsRequest>(),
	                UDP_MESSAGE);
	registerRequest(std::make_shared<ShowRecordRequest>(), UDP_MESSAGE);
	registerRequest(std::make_shared<OpenAuctionRequest>(), TCP_MESSAGE);
	registerRequest(std::make_shared<CloseAuctionRequest>(), TCP_MESSAGE);
	registerRequest(std::make_shared<ShowAssetRequest>(), TCP_MESSAGE);
	registerRequest(std::make_shared<BidRequest>(), TCP_MESSAGE);
	registerRequest(std::make_shared<WrongRequestUDP>(), TCP_MESSAGE);
	registerRequest(std::make_shared<WrongRequestTCP>(), TCP_MESSAGE);
}

/**
 * @brief  Calls the request handler for the message passed as parameter.
 * @param  message: Message to be handled.
 * @param  server: Server instance.
 * @param  address: Address of the client.
 * @param  type: Type of the message. Can be UDP_MESSAGE or TCP_MESSAGE.
 * @retval None
 */
void RequestManager::callHandlerRequest(MessageAdapter &message, Server &server,
                                        Address &address, int type) {
	std::string rec_proto_code = message.getn(PROTOCOL_SIZE);
	if (type == UDP_MESSAGE) {
		auto handler = _udp_handlers.find(rec_proto_code);
		if (handler == _udp_handlers.end()) {
			handler = _udp_handlers.find(CODE_ERROR);
		}
		handler->second->handle(message, server, address);
	} else {
		auto handler = _tcp_handlers.find(rec_proto_code);
		if (handler == _tcp_handlers.end()) {
			handler = _tcp_handlers.find(CODE_ERROR);
		}
		handler->second->handle(message, server, address);
	}
}

// -------------------------------------
// | Processing UDP and TCP			   |
// -------------------------------------

/**
 * @brief  Processes the UDP messages received by the server (Parent Process).
 * @param  server: Server instance.
 * @param  manager: Request manager instance.
 * @retval None
 */
void processUDP(Server &server, RequestManager &manager) {
	int ex_trial = 0;
	std::cout << "[UDP] Started UDP server." << std::endl;
	while (true) {
		try {
			wait_for_udp_message(server, manager);
			ex_trial = 0;
		} catch (UnknownHandlerException &e) {
			std::cerr << "[UDP] Unknown handler for message." << std::endl;
		} catch (MessageBuildingException &e) {
			std::cerr << "[UDP] Message couldn't be built to be sent."
					  << std::endl;
		} catch (std::exception &e) {
			std::cerr << "[UDP] Exception: " << e.what() << std::endl;
		} catch (...) {
			std::cerr
				<< "[UDP] Encountered unrecoverable error while running the "
				   "application. Retrying..."
				<< std::endl;
			ex_trial++;
		}
		if (ex_trial >= EXCEPTION_RETRY_MAX) {
			std::cerr << "[UDP] Max trials reached, shutting down..."
					  << std::endl;
			break;
		}
	}
}

/**
 * @brief  Processes the TCP messages received by the server (Child Process).
 * @param  server: Server instance.
 * @param  manager: Request manager instance.
 * @param  addr_from: Address of the client.
 * @param  connection_fd: File descriptor of the connection.
 * @retval None
 */
void processTCPChild(Server &server, RequestManager &manager, Address addr_from,
                     int connection_fd) {
	try {
		// Close parent listening socket
		close(server._tcp_socket_fd);

		// Set timeout for read and write in the socket
		struct timeval read_timeout;
		read_timeout.tv_sec = TCP_READ_TIMEOUT_SECONDS;
		read_timeout.tv_usec = 0;
		if (setsockopt(connection_fd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout,
		               sizeof(read_timeout)) < 0) {
			throw UnrecoverableException(
				"[TCP] Failed to set TCP read timeout socket option");
		}
		struct timeval write_timeout;
		write_timeout.tv_sec = TCP_WRITE_TIMEOUT_SECONDS;
		write_timeout.tv_usec = 0;
		if (setsockopt(connection_fd, SOL_SOCKET, SO_SNDTIMEO, &write_timeout,
		               sizeof(write_timeout)) < 0) {
			throw UnrecoverableException(
				"[TCP] Failed to set TCP write timeout socket option");
		}

		// Set up adapter
		addr_from.socket = connection_fd;
		TcpMessage message(connection_fd);

		// Call handler
		manager.callHandlerRequest(message, server, addr_from, TCP_MESSAGE);
		close(connection_fd);

		// Exit child process
		exit(EXIT_SUCCESS);
	} catch (std::exception &e) {
		printError("Handling tcp request. Child process exiting.");
		exit(EXIT_FAILURE);
	}
}

/**
 * @brief  Processes the TCP messages received by the server (Parent Process).
 * @param  server: Server instance.
 * @param  manager: Request manager instance.
 * @retval None
 */
void processTCP(Server &server, RequestManager &manager) {
	if (listen(server._tcp_socket_fd, TCP_MAX_QUEUE_SIZE) < 0) {
		perror("Error while executing listen");
		return;
	}
	std::cout << "[TCP] Started TCP server." << std::endl;

	uint32_t ex_trial = 0;
	while (true) {
		try {
			wait_for_tcp_message(server, manager);
			ex_trial = 0;
		} catch (std::exception &e) {
			std::cerr
				<< "[TCP] Encountered unrecoverable error while running the "
				   "server. Retrying..."
				<< std::endl
				<< e.what() << std::endl;
			ex_trial++;
		}
		if (ex_trial >= EXCEPTION_RETRY_MAX) {
			std::cerr << "[TCP] Max trials reached, shutting down..."
					  << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}

// -------------------------------------
// | Wait for TCP and UDP messages.	   |
// -------------------------------------

/**
 * @brief  Waits for a UDP message to be received by the server.
 * @param  server: Server instance.
 * @param  manager: Request manager instance.
 * @retval None
 */
void wait_for_udp_message(Server &server, RequestManager &manager) {
	Address addr_from;
	std::stringstream stream;
	char buffer[UDP_SOCKET_BUFFER_LEN];

	addr_from.size = sizeof(addr_from.addr);
	ssize_t n = recvfrom(server._udp_socket_fd, buffer, SOCKET_BUFFER_LEN, 0,
	                     (struct sockaddr *) &addr_from.addr, &addr_from.size);
	if (n == -1) {
		if (sig_int) {
			terminate(server, UDP_MESSAGE);
		}
		throw UnrecoverableException(
			"Failed to receive UDP message (recvfrom)");
	}
	stream.write(buffer, n);

	// Set up adapter
	addr_from.socket = server._udp_socket_fd;
	StreamMessage message(stream);

	// Call handler
	manager.callHandlerRequest(message, server, addr_from, UDP_MESSAGE);
	return;
}

/**
 * @brief  Waits for a TCP message to be received by the server.
 * @param  server: Server instance.
 * @param  manager: Request manager instance.
 * @retval None
 */
void wait_for_tcp_message(Server &server, RequestManager &manager) {
	Address addr_from;

	addr_from.size = sizeof(addr_from.addr);
	int connection_fd =
		accept(server._tcp_socket_fd, (struct sockaddr *) &addr_from.addr,
	           &addr_from.size);
	if (connection_fd < 0) {
		if (sig_int) {
			terminate(server, TCP_MESSAGE);
		}
		if (errno == EAGAIN) {  // timeout, just go around and keep listening
			return;
		}
		throw UnrecoverableException("[ERROR] Failed to accept a connection");
	}

	// Received message
	// Set timeout for read and write in the accepted socket
	struct timeval read_timeout;
	read_timeout.tv_sec = TCP_READ_TIMEOUT_SECONDS;
	read_timeout.tv_usec = 0;
	if (setsockopt(connection_fd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout,
	               sizeof(read_timeout)) < 0) {
		throw UnrecoverableException(
			"Failed to set TCP read timeout socket option");
	}

	try {
		// Delegate connection to child process
		pid_t pid = fork();
		if (pid < 0) {
			throw UnrecoverableException(
				"[ERROR] Failed to fork process. Couldn't delegate TCP "
				"connection to worker process.");
		} else if (pid == 0) {
			// Child process
			processTCPChild(server, manager, addr_from, connection_fd);
		} else {
			// Parent process
			close(connection_fd);
		}
	} catch (std::exception &e) {
		close(connection_fd);
		throw UnrecoverableException(
			std::string("Failed to delegate connection to child: ") + e.what() +
			"\nClosing connection.");
	}
}

// -------------------------------------
// | Main							   |
// -------------------------------------

int main(int argc, char *argv[]) {
	Server server(argc, argv);
	RequestManager requestManager;
	requestManager.registerRequestHandlers();

	pid_t c_pid = fork();
	if (c_pid == 0) {
		processUDP(server, requestManager);
	} else if (c_pid == -1) {
		std::cerr << "[ERROR] Failed to fork process." << std::endl;
		exit(EXIT_FAILURE);
	} else {
		processTCP(server, requestManager);
		std::cout << "[QUIT] Shutting Down. 11" << std::endl;
	}
	return EXIT_SUCCESS;
}