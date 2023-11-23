#include "client.hpp"
void Client::configClient(int argc, char *argv[]) {
	int opt;

	while ((opt = getopt(argc, argv, "hn:p:")) != -1) {
		switch (opt) {
			case 'n':
				this->_hostname = std::string(optarg);
				break;
			case 'p':
				this->_port = std::string(optarg);
				break;
			default:
				std::cout << "[ERROR] Config error." << std::endl;
				exit(EXIT_FAILURE);
		}
	}

	if (verify_port_number(_port) == -1) {
		exit(EXIT_FAILURE);
	};
}

Client::Client(int argc, char *argv[]) {
	configClient(argc, argv);
	// Create a UDP socket
	if ((this->_udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		throw SocketException();
	}
	this->resolveServerAddress(_hostname, _port);
}

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

void Client::sendUdpMessageAndAwaitReply(ProtocolMessage &out_message,
                                         ProtocolMessage &in_message) {
	int triesLeft = UDP_MAX_TRIES;
	while (triesLeft > 0) {
		--triesLeft;
		try {
			this->sendUdpMessage(out_message);
			this->waitForUdpMessage(in_message);
			return;
		} catch (ConnectionTimeoutException &e) {
			if (triesLeft == 0) {
				std::cout << "[ERROR] Couldn't send message" << std::endl;
			}
		}
	}
}

void Client::sendUdpMessage(ProtocolMessage &message) {
	send_message(message, _udp_socket_fd, _server_udp_addr->ai_addr,
	             _server_udp_addr->ai_addrlen);
}

void Client::waitForUdpMessage(ProtocolMessage &message) {
	await_message(message, _udp_socket_fd);
}

void Client::login(int user_id) {
	this->_user_id = user_id;
}

void Client::logout() {
	this->_user_id = LOGGED_OUT;
}

int Client::isLoggedIn() {
	return _user_id > -1;
}

int Client::getLoggedInUser() {
	return _user_id;
}
