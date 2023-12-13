#include "output.hpp"

#include <arpa/inet.h>

#include <iostream>

#include "server.hpp"
#include "shared/protocol.hpp"

// -----------------------------------
// | BASIC PRINTS					 |
// -----------------------------------

void printError(std::string message) {
	std::cerr << "[ERROR] " << message << std::endl;
}

void printInfo(std::string message) {
	std::cout << "[INFO] " << message << std::endl;
}

void printRequest(std::string message) {
	std::cout << "[REQUEST] " << message << std::endl;
}

// -----------------------------------
// | VERBOSE MODE (-v)				 |
// -----------------------------------

void printAddressIncomingRequest(Address &addr_from) {
	char addr_str[INET_ADDRSTRLEN + 1] = {0};
	inet_ntop(AF_INET, &addr_from.addr.sin_addr, addr_str, INET_ADDRSTRLEN);
	std::string message = "Receiving incoming UDP request from ";
	message += addr_str;
	message += ":";
	message += std::to_string(ntohs(addr_from.addr.sin_port));
	printInfo(message);
}

void printLoginRequest(ClientLoginUser request) {
	std::string message = "'LOGIN':\n\t-> User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n\t-> Password: ";
	for (int i = 0; i < request.password.length(); i++) {
		message += "*";
	};
	printInfo(message);
}