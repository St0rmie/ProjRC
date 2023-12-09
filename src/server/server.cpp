#include "server.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "handlers.hpp"

void RequestManager::registerRequest(std::shared_ptr<RequestHandler> handler,
                                     int type) {
	if (type == UDP_MESSAGE) {
		this->_udp_handlers.insert({handler->_protocol_code, handler});
	} else {
		this->_tcp_handlers.insert({handler->_protocol_code, handler});
	}
}

void RequestManager::registerRequestHandlers() {
	this->registerRequest(std::make_shared<LoginRequest>(), UDP_MESSAGE);
	this->registerRequest(std::make_shared<LogoutRequest>(), UDP_MESSAGE);
	this->registerRequest(std::make_shared<UnregisterRequest>(), UDP_MESSAGE);
	this->registerRequest(std::make_shared<ListAllAuctionsRequest>(),
	                      UDP_MESSAGE);
	this->registerRequest(std::make_shared<ListBiddedAuctionsRequest>(),
	                      UDP_MESSAGE);
	this->registerRequest(std::make_shared<ListStartedAuctionsRequest>(),
	                      UDP_MESSAGE);
	this->registerRequest(std::make_shared<ShowRecordRequest>(), UDP_MESSAGE);
	this->registerRequest(std::make_shared<OpenAuctionRequest>(), TCP_MESSAGE);
	this->registerRequest(std::make_shared<CloseAuctionRequest>(), TCP_MESSAGE);
	this->registerRequest(std::make_shared<ShowAssetRequest>(), TCP_MESSAGE);
	this->registerRequest(std::make_shared<BidRequest>(), TCP_MESSAGE);
}

void RequestManager::callHandlerRequest(MessageAdapter &message, Server &server,
                                        Address &address, int type) {
	std::string rec_proto_code = message.getn(PROTOCOL_SIZE);
	if (type == UDP_MESSAGE) {
		auto handler = this->_udp_handlers.find(rec_proto_code);
		if (handler == this->_udp_handlers.end()) {
			throw UnknownHandlerException();
			return;
		}
		handler->second->handle(message, server, address);
	} else {
		auto handler = this->_tcp_handlers.find(rec_proto_code);
		if (handler == this->_tcp_handlers.end()) {
			throw UnknownHandlerException();
			return;
		}
		handler->second->handle(message, server, address);
	}
}

void processUDP(Server &server, RequestManager &manager) {
	int ex_trial;
	while (true) {
		try {
			wait_for_udp_message(server, manager);
			ex_trial = 0;
		} catch (std::exception &e) {
			std::cerr << "Encountered unrecoverable error while running the "
						 "application. Retrying..."
					  << std::endl
					  << e.what() << std::endl;
			ex_trial++;
		} catch (...) {
			std::cerr << "Encountered unrecoverable error while running the "
						 "application. Retrying..."
					  << std::endl;
			ex_trial++;
		}
		if (ex_trial >= EXCEPTION_RETRY_MAX) {
			std::cerr << "Max trials reached, shutting down..." << std::endl;
		}
	}
}

void wait_for_udp_message(Server &server, RequestManager &manager) {
	Address addr_from;
	std::stringstream stream;
	char buffer[UDP_SOCKET_BUFFER_LEN];

	addr_from.size = sizeof(addr_from.addr);
	ssize_t n = recvfrom(server.udp_socket_fd, buffer, SOCKET_BUFFER_LEN, 0,
	                     (struct sockaddr *) &addr_from.addr, &addr_from.size);
	if (n == -1) {
		throw UnrecoverableException(
			"Failed to receive UDP message (recvfrom)");
	}
	addr_from.socket = server.udp_socket_fd;

	char addr_str[INET_ADDRSTRLEN + 1] = {0};
	inet_ntop(AF_INET, &addr_from.addr.sin_addr, addr_str, INET_ADDRSTRLEN);
	std::cout << "Receiving incoming UDP message from " << addr_str << ":"
			  << ntohs(addr_from.addr.sin_port) << std::endl;

	stream.write(buffer, n);
	StreamMessage message(stream);
	manager.callHandlerRequest(message, server, addr_from, UDP_MESSAGE);

	return;
}

void wait_for_tcp_packet(Server &server, RequestManager &manager) {
	Address addr_from;

	addr_from.size = sizeof(addr_from.addr);
	int connection_fd =
		accept(server.tcp_socket_fd, (struct sockaddr *) &addr_from.addr,
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

	char addr_str[INET_ADDRSTRLEN + 1] = {0};
	inet_ntop(AF_INET, &addr_from.addr.sin_addr, addr_str, INET_ADDRSTRLEN);
	std::cout << "Receiving incoming TCP connection from " << addr_str << ":"
			  << ntohs(addr_from.addr.sin_port) << std::endl;

	try {
		// pool.delegateConnection(connection_fd);
	} catch (std::exception &e) {
		close(connection_fd);
		throw UnrecoverableException(
			std::string("Failed to delegate connection to worker: ") +
			e.what() + "\nClosing connection.");
	}
}

void processTCP(Server &server, RequestManager &manager) {
	if (listen(server.tcp_socket_fd, TCP_MAX_QUEUE_SIZE) < 0) {
		perror("Error while executing listen");
		return;
	}

	uint32_t ex_trial = 0;
	while (true) {
		try {
			wait_for_tcp_packet(server, manager);
			ex_trial = 0;
		} catch (std::exception &e) {
			std::cerr << "Encountered unrecoverable error while running the "
						 "application. Retrying..."
					  << std::endl
					  << e.what() << std::endl;
			ex_trial++;
		} catch (...) {
			std::cerr << "Encountered unrecoverable error while running the "
						 "application. Retrying..."
					  << std::endl;
			ex_trial++;
		}
		if (ex_trial >= EXCEPTION_RETRY_MAX) {
			std::cerr << "Max trials reached, shutting down..." << std::endl;
			return;
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
		// processTCP(server, requestManager);
		std::cout << "[QUIT] Shutting Down." << std::endl;
	}
	return EXIT_SUCCESS;
}