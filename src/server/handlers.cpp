#include "handlers.hpp"

#include "protocol.hpp"

void RequestManager::registerRequest(std::shared_ptr<RequestHandler> handler,
                                     int type) {
	if (type == UDP_MESSAGE) {
		this->_udp_handlers.insert({handler->_protocol_code, handler});
	} else {
		this->_tcp_handlers.insert({handler->_protocol_code, handler});
	}
}

void RequestManager::registerRequestHandlers(RequestManager &manager) {
	manager.registerRequest(std::make_shared<LoginRequest>(), UDP_MESSAGE);
	manager.registerRequest(std::make_shared<LogoutRequest>(), UDP_MESSAGE);
	manager.registerRequest(std::make_shared<UnregisterRequest>(), UDP_MESSAGE);
	manager.registerRequest(std::make_shared<ListAllAuctionsRequest>(),
	                        UDP_MESSAGE);
	manager.registerRequest(std::make_shared<ListBiddedAuctionsRequest>(),
	                        UDP_MESSAGE);
	manager.registerRequest(std::make_shared<ListStartedAuctionsRequest>(),
	                        UDP_MESSAGE);
	manager.registerRequest(std::make_shared<ShowRecordRequest>(), UDP_MESSAGE);
	manager.registerRequest(std::make_shared<OpenAuctionRequest>(),
	                        TCP_MESSAGE);
	manager.registerRequest(std::make_shared<CloseAuctionRequest>(),
	                        TCP_MESSAGE);
	manager.registerRequest(std::make_shared<ShowAssetRequest>(), TCP_MESSAGE);
	manager.registerRequest(std::make_shared<BidRequest>(), TCP_MESSAGE);
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