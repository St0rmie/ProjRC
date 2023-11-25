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
	await_udp_message(message, _udp_socket_fd);
}

void Client::sendTcpMessageAndAwaitReply(ProtocolMessage &out_message,
                                         ProtocolMessage &in_message) {
	try {
		openTcpSocket();
		sendTcpMessage(out_message);
		waitForTcpMessage(in_message);
	} catch (...) {
		closeTcpSocket();
	}
	closeTcpSocket();
}

void Client::openTcpSocket() {
	this->_tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_tcp_socket_fd == -1) {
		throw SocketException();
	}

	struct timeval read_timeout;
	read_timeout.tv_sec = TCP_READ_TIMEOUT_SECONDS;
	read_timeout.tv_usec = TCP_READ_TIMEOUT_USECONDS;
	int errcode = setsockopt(this->_tcp_socket_fd, SOL_SOCKET, SO_RCVTIMEO,
	                         &read_timeout, sizeof(read_timeout));
	if (errcode < 0) {
		throw SocketException();
	}

	struct timeval write_timeout;
	write_timeout.tv_sec = TCP_WRITE_TIMEOUT_SECONDS;
	write_timeout.tv_usec = TCP_WRITE_TIMEOUT_USECONDS;
	errcode = setsockopt(this->_tcp_socket_fd, SOL_SOCKET, SO_SNDTIMEO,
	                     &write_timeout, sizeof(write_timeout));
	if (errcode < 0) {
		throw SocketException();
	}
}

void Client::sendTcpMessage(ProtocolMessage &message) {
	ssize_t n = connect(_tcp_socket_fd, _server_tcp_addr->ai_addr,
	                    _server_tcp_addr->ai_addrlen);
	if (n == -1) {
		throw ConnectionTimeoutException();
	}
	const char *message_str = message.buildMessage().str().c_str();
	size_t bytes_to_send = strlen(message_str);
	size_t bytes_sent = 0;
	while (bytes_sent < bytes_to_send) {
		ssize_t sent = write(_tcp_socket_fd, message_str + bytes_sent,
		                     bytes_to_send - bytes_sent);
		if (sent < 0) {
			throw MessageSendException();
		}
		bytes_sent += sent;
	}
}

void Client::waitForTcpMessage(ProtocolMessage &message) {
	await_tcp_message(message, _tcp_socket_fd);
};

void Client::closeTcpSocket() {
	if (close(this->_tcp_socket_fd) != 0) {
		if (errno == EBADF) {
			// was already closed
			return;
		}
		throw SocketException();
	}
};

void Client::login(uint32_t user_id, std::string password) {
	this->_user_id = user_id;
	this->_password = password;
}

void Client::logout() {
	this->_user_id = LOGGED_OUT;
	this->_password = "";
}

bool Client::isLoggedIn() {
	return (_user_id != LOGGED_OUT);
}

int Client::getLoggedInUser() {
	return _user_id;
}

std::string Client::getPassword() {
	return _password;
}

void printError(std::string error_description) {
	std::cout << "[ERROR] " << error_description << std::endl;
}

void printSuccess(std::string success_description) {
	std::cout << "[SUCCESS] " << success_description << std::endl;
}