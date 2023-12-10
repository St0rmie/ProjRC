#include "handlers.hpp"

#include "shared/protocol.hpp"

void LoginRequest::handle(MessageAdapter &message, Server &server,
                          Address &address) {
	ClientLoginUser message_in;
	ServerLoginUser message_out;
	try {
		message_in.readMessage(message);
		std::string user_id = std::to_string(message_in.user_id);
		std::cerr << "password: " << message_in.password << std::endl;
		int res = server._database.LoginUser(user_id, message_in.password);
		switch (res) {
			case -1:
				message_out.status = ServerLoginUser::status::NOK;
				break;

			case 0:
				message_out.status = ServerLoginUser::status::OK;
				break;

			case 2:
				message_out.status = ServerLoginUser::status::REG;
				break;
		}

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

void ShowRecordRequest::handle(MessageAdapter &message, Server &server,
                               Address &address) {
	ClientShowRecord message_in;
	ServerShowRecord message_out;
	try {
		message_in.readMessage(message);

		// DATABASE OPERATIONS
	} catch (...) {
		std::cout << "Failed to handle login request." << std::endl;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size);
}

void OpenAuctionRequest::handle(MessageAdapter &message, Server &server,
                                Address &address) {
	ClientOpenAuction message_in;
	ServerOpenAuction message_out;
}

void CloseAuctionRequest::handle(MessageAdapter &message, Server &server,
                                 Address &address) {
	ClientCloseAuction message_in;
	ServerCloseAuction message_out;
}

void ShowAssetRequest::handle(MessageAdapter &message, Server &server,
                              Address &address) {
	ClientShowAsset message_in;
	ServerShowAsset message_out;
}

void BidRequest::handle(MessageAdapter &message, Server &server,
                        Address &address) {
	ClientBid message_in;
	ServerBid message_out;
}