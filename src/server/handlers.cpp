#include "handlers.hpp"

#include "output.hpp"
#include "shared/protocol.hpp"

void LoginRequest::handle(MessageAdapter &message, Server &server,
                          Address &address) {
	ClientLoginUser message_in;
	ServerLoginUser message_out;
	try {
		message_in.readMessage(message);
		std::string user_id = std::to_string(message_in.user_id);
		int res = server._database.LoginUser(user_id, message_in.password);
		switch (res) {
			case DB_LOGIN_NOK:
				message_out.status = ServerLoginUser::status::NOK;
				break;

			case DB_LOGIN_OK:
				message_out.status = ServerLoginUser::status::OK;
				break;

			case DB_LOGIN_REGISTER:
				message_out.status = ServerLoginUser::status::REG;
				break;
		}
	} catch (InvalidMessageException &e) {
		message_out.status = ServerLoginUser::status::ERR;
	} catch (...) {
		printError("Failed to handle 'LOGIN' request.");
		return;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size,
	                 server._verbose);
}

void LogoutRequest::handle(MessageAdapter &message, Server &server,
                           Address &address) {
	ClientLogout message_in;
	ServerLogout message_out;
	try {
		message_in.readMessage(message);
		std::string user_id = std::to_string(message_in.user_id);
		int res = server._database.Logout(user_id, message_in.password);
		switch (res) {
			case DB_LOGOUT_UNREGISTERED:
				message_out.status = ServerLogout::status::UNR;
				break;

			case DB_LOGOUT_OK:
				message_out.status = ServerLogout::status::OK;
				break;

			case DB_LOGOUT_NOK:
				message_out.status = ServerLogout::status::NOK;
				break;
		}
	} catch (InvalidMessageException &e) {
		message_out.status = ServerLogout::status::ERR;
	} catch (...) {
		printError("Failed to handle 'LOGOUT' request.");
		return;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size,
	                 server._verbose);
}

void UnregisterRequest::handle(MessageAdapter &message, Server &server,
                               Address &address) {
	ClientUnregister message_in;
	ServerUnregister message_out;
	try {
		message_in.readMessage(message);
		std::string user_id = std::to_string(message_in.user_id);
		int res = server._database.Unregister(user_id, message_in.password);
		switch (res) {
			case DB_UNREGISTER_UNKNOWN:
				message_out.status = ServerUnregister::status::UNR;
				break;

			case DB_UNREGISTER_OK:
				message_out.status = ServerUnregister::status::OK;
				break;

			case DB_UNREGISTER_NOK:
				message_out.status = ServerUnregister::status::NOK;
				break;
		}
	} catch (InvalidMessageException &e) {
		message_out.status = ServerUnregister::status::ERR;
	} catch (...) {
		printError("Failed to handle 'UNREGISTER' request.");
		return;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size,
	                 server._verbose);
}

void ListAllAuctionsRequest::handle(MessageAdapter &message, Server &server,
                                    Address &address) {
	ClientListAllAuctions message_in;
	ServerListAllAuctions message_out;
	try {
		message_in.readMessage(message);

		AuctionList a_list = server._database.List();
		size_t list_size = a_list.size();
		list_size = 2;

		if (list_size == 0) {
			message_out.status = ServerListAllAuctions::status::NOK;
		} else {
			message_out.status = ServerListAllAuctions::status::OK;
			for (AuctionListing a : a_list) {
				int state = a.active ? 1 : 0;
				std::string auction_str = a.a_id + " " + std::to_string(state);
				message_out.auctions.push_back(auction_str);
			}
		}
	} catch (InvalidMessageException &e) {
		message_out.status = ServerListAllAuctions::status::ERR;
	} catch (...) {
		printError("Failed to handle 'LIST' request.");
		return;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size,
	                 server._verbose);
}

void ListBiddedAuctionsRequest::handle(MessageAdapter &message, Server &server,
                                       Address &address) {
	ClientListBiddedAuctions message_in;
	ServerListBiddedAuctions message_out;
	try {
		message_in.readMessage(message);

		std::string user_id = std::to_string(message_in.user_id);
		AuctionList a_list = server._database.MyBids(user_id);
		size_t list_size = a_list.size();
		list_size = 2;

		if (server._database.CheckUserLoggedIn(user_id) != 0) {
			message_out.status = ServerListBiddedAuctions::status::NLG;
		} else if (list_size == 0) {
			message_out.status = ServerListBiddedAuctions::status::NOK;
		} else {
			message_out.status = ServerListBiddedAuctions::status::OK;
			for (AuctionListing a : a_list) {
				int state = a.active ? 1 : 0;
				std::string auction_str = a.a_id + " " + std::to_string(state);
				message_out.auctions.push_back(auction_str);
			}
		}
	} catch (InvalidMessageException &e) {
		message_out.status = ServerListBiddedAuctions::status::ERR;
	} catch (...) {
		printError("Failed to handle 'LIST MY BIDS' request.");
		return;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size,
	                 server._verbose);
}

void ListStartedAuctionsRequest::handle(MessageAdapter &message, Server &server,
                                        Address &address) {
	ClientListStartedAuctions message_in;
	ServerListStartedAuctions message_out;
	try {
		message_in.readMessage(message);

		std::string user_id = std::to_string(message_in.user_id);
		AuctionList a_list = server._database.MyAuctions(user_id);
		size_t list_size = a_list.size();
		list_size = 2;

		if (server._database.CheckUserLoggedIn(user_id) != 0) {
			message_out.status = ServerListStartedAuctions::status::NLG;
		} else if (list_size == 0) {
			message_out.status = ServerListStartedAuctions::status::NOK;
		} else {
			message_out.status = ServerListStartedAuctions::status::OK;
			for (AuctionListing a : a_list) {
				int state = a.active ? 1 : 0;
				std::string auction_str = a.a_id + " " + std::to_string(state);
				message_out.auctions.push_back(auction_str);
			}
		}
	} catch (InvalidMessageException &e) {
		message_out.status = ServerListStartedAuctions::status::ERR;
	} catch (...) {
		printError("Failed to handle 'LIST MY AUCTIONS' request.");
		return;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size,
	                 server._verbose);
}

void ShowRecordRequest::handle(MessageAdapter &message, Server &server,
                               Address &address) {
	ClientShowRecord message_in;
	ServerShowRecord message_out;
	try {
		message_in.readMessage(message);

		std::string auction_id =
			convert_auction_id_to_str(message_in.auction_id);
		Record record = server._database.ShowRecord(auction_id);

	} catch (InvalidMessageException &e) {
		message_out.status = ServerShowRecord::status::ERR;
	} catch (...) {
		printError("Failed to handle 'SHOW RECORD' request.");
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size,
	                 server._verbose);
}

void OpenAuctionRequest::handle(MessageAdapter &message, Server &server,
                                Address &address) {
	ClientOpenAuction message_in;
	ServerOpenAuction message_out;

	try {
		message_in.readMessage(message);
		std::string user_id = convert_user_id_to_str(message_in.user_id);
		std::string start_value = std::to_string(message_in.start_value);
		std::string timeactive = std::to_string(message_in.timeactive);
		int aid = server._database.Open(
			user_id, message_in.name, message_in.password,
			message_in.assetf_name, start_value, timeactive, message_in.fsize,
			message_in.fdata);

		if (aid > 0) {
			message_out.status = ServerOpenAuction::status::OK;
			message_out.auction_id = (uint32_t) aid;
		} else if (aid == DB_OPEN_NOT_LOGGED_IN) {
			message_out.status = ServerOpenAuction::status::NLG;
		} else {
			message_out.status = ServerOpenAuction::status::NOK;
		}

	} catch (InvalidMessageException &e) {
		message_out.status = ServerOpenAuction::status::ERR;
	} catch (...) {
		printError("Failed to handle 'OPEN AUCTION' request.");
		return;
	}

	send_tcp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size,
	                 server._verbose);
}

void CloseAuctionRequest::handle(MessageAdapter &message, Server &server,
                                 Address &address) {
	ClientCloseAuction message_in;
	ServerCloseAuction message_out;

	try {
		message_in.readMessage(message);
		std::string user_id = convert_user_id_to_str(message_in.user_id);
		std::string auction_id =
			convert_auction_id_to_str(message_in.auction_id);

	} catch (InvalidMessageException &e) {
		message_out.status = ServerCloseAuction::status::ERR;
	} catch (...) {
		printError("Failed to handle 'CLOSE AUCTION' request.");
		return;
	}

	send_tcp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size,
	                 server._verbose);
}

void ShowAssetRequest::handle(MessageAdapter &message, Server &server,
                              Address &address) {
	ClientShowAsset message_in;
	ServerShowAsset message_out;

	try {
		message_in.readMessage(message);

	} catch (InvalidMessageException &e) {
		message_out.status = ServerShowAsset::status::ERR;
	} catch (...) {
		printError("Failed to handle 'SHOW ASSET' request.");
		return;
	}

	send_tcp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size,
	                 server._verbose);
}

void BidRequest::handle(MessageAdapter &message, Server &server,
                        Address &address) {
	ClientBid message_in;
	ServerBid message_out;

	try {
		message_in.readMessage(message);

	} catch (InvalidMessageException &e) {
		message_out.status = ServerBid::status::ERR;
	} catch (...) {
		printError("Failed to handle 'BID' request.");
		return;
	}

	send_tcp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size,
	                 server._verbose);
}