#include "server.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "handlers.hpp"

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
		throw UnrecoverableException();
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

void processTCP(Server &server, RequestManager &manager) {}

int main(int argc, char *argv[]) {
	Server server(argc, argv);
	RequestManager requestManager;
	registerRequestHandlers(requestManager);

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