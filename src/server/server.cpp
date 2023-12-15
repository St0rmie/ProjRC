#include "server.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <csignal>

#include "handlers.hpp"
#include "output.hpp"

void Server::setupSignalHandlers() {
	// ignore SIGPIPE
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
}

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

Server::Server(int argc, char *argv[]) {
	configServer(argc, argv);
	// Create a UDP socket
	if ((_udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		throw UnrecoverableException("[ERROR] Couldn't open socket");
	}
	// Creates base for database
	_database.CreateBaseDir();
	// Setup sockets
	setup_sockets();
	resolveServerAddress(_port);
}

void Server::setup_sockets() {
	// Create a UDP socket
	if ((_udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		throw UnrecoverableException("Failed to create a UDP socket");
	}
	// Force reuse of the socket.
	const int enable_udp = 1;
	if (setsockopt(_udp_socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable_udp,
	               sizeof(int)) < 0) {
		throw UnrecoverableException(
			"Failed to set UDP reuse address socket option");
	}

	// Create a TCP socket
	if ((_tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		throw UnrecoverableException("Failed to create a TCP socket");
	}
	const int enable_tcp = 1;

	// Force reuse of the socket.
	if (setsockopt(_tcp_socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable_tcp,
	               sizeof(int)) < 0) {
		throw UnrecoverableException(
			"Failed to set TCP reuse address socket option");
	}
}

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
			std::string("Failed to get address for UDP connection: ") +
			gai_strerror(addr_res));
	}
	// bind socket
	if (bind(_udp_socket_fd, _server_udp_addr->ai_addr,
	         _server_udp_addr->ai_addrlen)) {
		throw UnrecoverableException("Failed to bind UDP address.");
	}

	// Get TCP address
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;        // IPv4
	hints.ai_socktype = SOCK_STREAM;  // TCP socket
	hints.ai_flags = AI_PASSIVE;      // Listen on 0.0.0.0
	if ((addr_res =
	         getaddrinfo(NULL, port.c_str(), &hints, &_server_tcp_addr)) != 0) {
		throw UnrecoverableException(
			std::string("Failed to get address for TCP connection: ") +
			gai_strerror(addr_res));
	}

	if (bind(_tcp_socket_fd, _server_tcp_addr->ai_addr,
	         _server_tcp_addr->ai_addrlen)) {
		throw UnrecoverableException("Failed to bind TCP address");
	}

	std::cout << "Listening for connections on port " << port << std::endl;
}

void RequestManager::registerRequest(std::shared_ptr<RequestHandler> handler,
                                     int type) {
	if (type == UDP_MESSAGE) {
		_udp_handlers.insert({handler->_protocol_code, handler});
	} else {
		_tcp_handlers.insert({handler->_protocol_code, handler});
	}
}

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

void processUDP(Server &server, RequestManager &manager) {
	int ex_trial = 0;
	while (true) {
		try {
			wait_for_udp_message(server, manager);
			ex_trial = 0;
		} catch (UnknownHandlerException &e) {
			std::cerr << "Unknown handler for message." << std::endl;
		} catch (MessageBuildingException &e) {
			std::cerr << "Message couldn't be built to be sent." << std::endl;
		} catch (std::exception &e) {
			std::cerr << "EXCEPTION: " << e.what() << std::endl;
		} catch (...) {
			std::cerr << "Encountered unrecoverable error while running the "
						 "application. Retrying..."
					  << std::endl;
			ex_trial++;
		}
		if (ex_trial >= EXCEPTION_RETRY_MAX) {
			std::cerr << "Max trials reached, shutting down..." << std::endl;
			break;
		}
	}
}

void wait_for_udp_message(Server &server, RequestManager &manager) {
	Address addr_from;
	std::stringstream stream;
	char buffer[UDP_SOCKET_BUFFER_LEN];

	addr_from.size = sizeof(addr_from.addr);
	ssize_t n = recvfrom(server._udp_socket_fd, buffer, SOCKET_BUFFER_LEN, 0,
	                     (struct sockaddr *) &addr_from.addr, &addr_from.size);
	if (n == -1) {
		throw UnrecoverableException(
			"Failed to receive UDP message (recvfrom)");
	}
	stream.write(buffer, n);
	addr_from.socket = server._udp_socket_fd;

	StreamMessage message(stream);
	manager.callHandlerRequest(message, server, addr_from, UDP_MESSAGE);

	return;
}

void wait_for_tcp_message(Server &server, RequestManager &manager) {
	Address addr_from;

	addr_from.size = sizeof(addr_from.addr);
	int connection_fd =
		accept(server._tcp_socket_fd, (struct sockaddr *) &addr_from.addr,
	           &addr_from.size);
	if (connection_fd < 0) {
		if (errno == EAGAIN) {  // timeout, just go around and keep listening
			return;
		}
		throw UnrecoverableException("[ERROR] Failed to accept a connection");
	}

	struct timeval read_timeout;
	read_timeout.tv_sec = TCP_READ_TIMEOUT_SECONDS;
	read_timeout.tv_usec = 0;
	if (setsockopt(connection_fd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout,
	               sizeof(read_timeout)) < 0) {
		throw UnrecoverableException(
			"Failed to set TCP read timeout socket option");
	}

	try {
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
			std::string("Failed to delegate connection to worker: ") +
			e.what() + "\nClosing connection.");
	}
}

void processTCPChild(Server &server, RequestManager &manager, Address addr_from,
                     int connection_fd) {
	try {
		close(server._tcp_socket_fd);

		struct timeval read_timeout;
		read_timeout.tv_sec = TCP_READ_TIMEOUT_SECONDS;
		read_timeout.tv_usec = 0;
		if (setsockopt(connection_fd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout,
		               sizeof(read_timeout)) < 0) {
			throw UnrecoverableException(
				"[ERROR] Failed to set TCP read timeout socket option");
		}
		struct timeval write_timeout;
		write_timeout.tv_sec = TCP_WRITE_TIMEOUT_SECONDS;
		write_timeout.tv_usec = 0;
		if (setsockopt(connection_fd, SOL_SOCKET, SO_SNDTIMEO, &write_timeout,
		               sizeof(write_timeout)) < 0) {
			throw UnrecoverableException(
				"[ERROR] Failed to set TCP write timeout socket option");
		}

		addr_from.socket = connection_fd;
		TcpMessage message(connection_fd);
		manager.callHandlerRequest(message, server, addr_from, TCP_MESSAGE);
		close(connection_fd);
		exit(EXIT_SUCCESS);
	} catch (std::exception &e) {
		printError("Handling tcp request. Child process exiting.");
		exit(EXIT_FAILURE);
	}
}

void processTCP(Server &server, RequestManager &manager) {
	if (listen(server._tcp_socket_fd, TCP_MAX_QUEUE_SIZE) < 0) {
		perror("Error while executing listen");
		return;
	}

	uint32_t ex_trial = 0;
	while (true) {
		try {
			wait_for_tcp_message(server, manager);
			ex_trial = 0;
		} catch (std::exception &e) {
			std::cerr << "Encountered unrecoverable error while running the "
						 "application. Retrying..."
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

int main(int argc, char *argv[]) {
	Server server(argc, argv);
	RequestManager requestManager;
	requestManager.registerRequestHandlers();

	pid_t c_pid = fork();
	if (c_pid == 0) {
		processUDP(server, requestManager);
	} else if (c_pid == -1) {
		std::cerr << "Failed to fork process." << std::endl;
		exit(EXIT_FAILURE);
	} else {
		processTCP(server, requestManager);
		std::cout << "[QUIT] Shutting Down." << std::endl;
	}
	return EXIT_SUCCESS;
}