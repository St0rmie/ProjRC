#include "handlers.hpp"

#include "protocol.hpp"

void LoginRequest::handle(MessageAdapter &message, Server &server,
                          Address &address) {
	ClientLoginUser message_in;
	ServerLoginUser message_out;
	try {
		message_in.readMessage(message);

		// DATABASE OPERATIONS
	} catch (...) {
		std::cout << "Failed to handle login request." << std::endl;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size);
}

void LogoutRequest::handle(MessageAdapter &message, Server &server,
                           Address &address) {
	ClientLogout message_in;
	ServerLogout message_out;
	try {
		message_in.readMessage(message);

		// DATABASE OPERATIONS
	} catch (...) {
		std::cout << "Failed to handle login request." << std::endl;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size);
}

void UnregisterRequest::handle(MessageAdapter &message, Server &server,
                               Address &address) {
	ClientUnregister message_in;
	ServerUnregister message_out;
	try {
		message_in.readMessage(message);

		// DATABASE OPERATIONS
	} catch (...) {
		std::cout << "Failed to handle login request." << std::endl;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size);
}

void ListAllAuctionsRequest::handle(MessageAdapter &message, Server &server,
                                    Address &address) {
	ClientListAllAuctions message_in;
	ServerListAllAuctions message_out;
	try {
		message_in.readMessage(message);

		// DATABASE OPERATIONS
	} catch (...) {
		std::cout << "Failed to handle login request." << std::endl;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size);
}

void ListBiddedAuctionsRequest::handle(MessageAdapter &message, Server &server,
                                       Address &address) {
	ClientListBiddedAuctions message_in;
	ServerListBiddedAuctions message_out;
	try {
		message_in.readMessage(message);

		// DATABASE OPERATIONS
	} catch (...) {
		std::cout << "Failed to handle login request." << std::endl;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size);
}

void ListStartedAuctionsRequest::handle(MessageAdapter &message, Server &server,
                                        Address &address) {
	ClientListStartedAuctions message_in;
	ServerListStartedAuctions message_out;
	try {
		message_in.readMessage(message);

		// DATABASE OPERATIONS
	} catch (...) {
		std::cout << "Failed to handle login request." << std::endl;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size);
}