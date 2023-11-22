#include "client.hpp"

Client::Client(std::string &hostname, std::string &port) {
	// Create a UDP socket
	if ((this->_udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		throw SocketException();
	}
	this->resolveServerAddress(hostname, port);
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
