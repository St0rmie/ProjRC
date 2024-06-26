#include "handlers.hpp"

#include "output.hpp"
#include "shared/protocol.hpp"

/**
 * @brief  Responsible for handling the login request and consult the database.
 * @param  &message: The adapter containing the raw received message.
 * @param  &server: Instance of the server.
 * @param  &address: The address to where the message should go.
 * @throws InvalidMessageException if the message is wrongly formatted or
 * conatins invalid contents.
 * @retval None
 */
void LoginRequest::handle(MessageAdapter &message, Server &server,
                          Address &address) {
	ClientLoginUser message_in;
	ServerLoginUser message_out;
	try {
		message_in.readMessage(message);
		if (server._verbose) {
			printAddressIncomingRequest(address);
			printInLoginRequest(message_in);
		}

		std::string user_id = std::to_string(message_in.user_id);

		// Access database
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

			default:
				throw InvalidMessageException();
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

/**
 * @brief  Responsible for handling the logout request and consult the database.
 * @param  &message: The adapter containing the raw received message.
 * @param  &server: Instance of the server.
 * @param  &address: The address to where the message should go.
 * @throws InvalidMessageException if the message is wrongly formatted or
 * conatins invalid contents.
 * @retval None
 */
void LogoutRequest::handle(MessageAdapter &message, Server &server,
                           Address &address) {
	ClientLogout message_in;
	ServerLogout message_out;
	try {
		message_in.readMessage(message);
		if (server._verbose) {
			printAddressIncomingRequest(address);
			printInLogoutRequest(message_in);
		}

		std::string user_id = std::to_string(message_in.user_id);

		// Access database
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

			default:
				throw InvalidMessageException();
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

/**
 * @brief  Responsible for handling the unregister request and consult the
 * database.
 * @param  &message: The adapter containing the raw received message.
 * @param  &server: Instance of the server.
 * @param  &address: The address to where the message should go.
 * @throws InvalidMessageException if the message is wrongly formatted or
 * conatins invalid contents.
 * @retval None
 */
void UnregisterRequest::handle(MessageAdapter &message, Server &server,
                               Address &address) {
	ClientUnregister message_in;
	ServerUnregister message_out;
	try {
		message_in.readMessage(message);
		if (server._verbose) {
			printAddressIncomingRequest(address);
			printInUnregisterRequest(message_in);
		}

		std::string user_id = std::to_string(message_in.user_id);

		// Access database
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

			default:
				throw InvalidMessageException();
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

/**
 * @brief  Responsible for handling the List All request and consult the
 * database.
 * @param  &message: The adapter containing the raw received message.
 * @param  &server: Instance of the server.
 * @param  &address: The address to where the message should go.
 * @throws InvalidMessageException if the message is wrongly formatted or
 * conatins invalid contents.
 * @retval None
 */
void ListAllAuctionsRequest::handle(MessageAdapter &message, Server &server,
                                    Address &address) {
	ClientListAllAuctions message_in;
	ServerListAllAuctions message_out;
	try {
		message_in.readMessage(message);
		if (server._verbose) {
			printAddressIncomingRequest(address);
			printInListAllRequest(message_in);
		}

		// Access database
		AuctionList a_list = server._database.List();

		if (a_list.size() == 0) {
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
	} catch (std::exception &e) {
		printError("Failed to handle 'LIST' request." + std::string(e.what()));
		return;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size,
	                 server._verbose);
}

/**
 * @brief  Responsible for handling the List My Bids request and consult the
 * database.
 * @param  &message: The adapter containing the raw received message.
 * @param  &server: Instance of the server.
 * @param  &address: The address to where the message should go.
 * @throws InvalidMessageException if the message is wrongly formatted or
 * conatins invalid contents.
 * @retval None
 */
void ListBiddedAuctionsRequest::handle(MessageAdapter &message, Server &server,
                                       Address &address) {
	ClientListBiddedAuctions message_in;
	ServerListBiddedAuctions message_out;
	try {
		message_in.readMessage(message);
		if (server._verbose) {
			printAddressIncomingRequest(address);
			printInListBiddedRequest(message_in);
		}

		std::string user_id = std::to_string(message_in.user_id);
		// Access database
		AuctionList a_list = server._database.MyBids(user_id);

		if (server._database.CheckUserLoggedIn(user_id) != 0) {
			message_out.status = ServerListBiddedAuctions::status::NLG;
		} else if (a_list.size() == 0) {
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
	} catch (std::exception &e) {
		printError("Failed to handle 'LIST MY BIDS' request." +
		           std::string(e.what()));
		return;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size,
	                 server._verbose);
}

/**
 * @brief  Responsible for handling the List My Auctions request and consult the
 * database.
 * @param  &message: The adapter containing the raw received message.
 * @param  &server: Instance of the server.
 * @param  &address: The address to where the message should go.
 * @throws InvalidMessageException if the message is wrongly formatted or
 * conatins invalid contents.
 * @retval None
 */
void ListStartedAuctionsRequest::handle(MessageAdapter &message, Server &server,
                                        Address &address) {
	ClientListStartedAuctions message_in;
	ServerListStartedAuctions message_out;
	try {
		message_in.readMessage(message);
		if (server._verbose) {
			printAddressIncomingRequest(address);
			printInListStartedRequest(message_in);
		}

		std::string user_id = std::to_string(message_in.user_id);
		// Access database
		AuctionList a_list = server._database.MyAuctions(user_id);

		if (server._database.CheckUserLoggedIn(user_id) != 0) {
			message_out.status = ServerListStartedAuctions::status::NLG;
		} else if (a_list.size() == 0) {
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
	} catch (std::exception &e) {
		printError("Failed to handle 'LIST MY AUCTIONS' request." +
		           std::string(e.what()));
		return;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size,
	                 server._verbose);
}

/**
 * @brief  Responsible for handling the Show Record request and consult the
 * database.
 * @param  &message: The adapter containing the raw received message.
 * @param  &server: Instance of the server.
 * @param  &address: The address to where the message should go.
 * @throws InvalidMessageException if the message is wrongly formatted or
 * conatins invalid contents.
 * @throws AuctionNotFound if the request is attempted on a non-existant
 * auction.
 * @retval None
 */
void ShowRecordRequest::handle(MessageAdapter &message, Server &server,
                               Address &address) {
	ClientShowRecord message_in;
	ServerShowRecord message_out;
	try {
		message_in.readMessage(message);
		if (server._verbose) {
			printAddressIncomingRequest(address);
			printInShowRecordRequest(message_in);
		}

		std::string auction_id =
			convert_auction_id_to_str(message_in.auction_id);

		// Access database
		AuctionRecord record = server._database.ShowRecord(auction_id);
		message_out.status = ServerShowRecord::status::OK;
		message_out.host_UID = static_cast<uint32_t>(stoi(record.host_id));
		message_out.auction_name = record.auction_name;
		message_out.asset_fname = record.asset_fname;
		message_out.start_value =
			static_cast<uint32_t>(stoi(record.start_value));
		message_out.start_date_time =
			convert_str_to_date(record.start_datetime);
		message_out.timeactive = static_cast<uint32_t>(stoi(record.timeactive));

		for (BidInfo b : record.list) {
			Bid bid;
			bid.bidder_UID = static_cast<uint32_t>(stoi(b.user_id));
			bid.bid_value = static_cast<uint32_t>(stoi(b.value));
			bid.bid_date_time = convert_str_to_date(b.current_date);
			bid.bid_sec_time = b.time_passed;
			message_out.bids.push_back(bid);
		}

		if (!record.active) {
			message_out.end_date_time =
				convert_str_to_date(record.end_datetime);
			message_out.end_sec_time = record.end_timeelapsed;
		}

	} catch (InvalidMessageException &e) {
		message_out.status = ServerShowRecord::status::ERR;
	} catch (AuctionNotFound &e) {
		message_out.status = ServerShowRecord::status::NOK;
	} catch (std::exception &e) {
		printError("Failed to handle 'SHOW RECORD' request." +
		           std::string(e.what()));
		return;
	}

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size,
	                 server._verbose);
}

/**
 * @brief  Responsible for handling the Open Auction request and consult the
 * database.
 * @param  &message: The adapter containing the raw received message.
 * @param  &server: Instance of the server.
 * @param  &address: The address to where the message should go.
 * @throws InvalidMessageException if the message is wrongly formatted or
 * conatins invalid contents.
 * @retval None
 */
void OpenAuctionRequest::handle(MessageAdapter &message, Server &server,
                                Address &address) {
	ClientOpenAuction message_in;
	ServerOpenAuction message_out;

	try {
		message_in.readMessage(message);
		if (server._verbose) {
			printAddressIncomingRequest(address);
			printInOpenAuctionRequest(message_in);
		}

		std::string user_id = convert_user_id_to_str(message_in.user_id);
		std::string start_value = std::to_string(message_in.start_value);
		std::string timeactive = std::to_string(message_in.timeactive);

		// Access database
		int aid = server._database.Open(
			user_id, message_in.name, message_in.password,
			message_in.assetf_name, start_value, timeactive, message_in.Fsize,
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
	} catch (std::exception &e) {
		printError("Failed to handle 'OPEN AUCTION' request." +
		           std::string(e.what()));
		return;
	}

	send_tcp_message(message_out, address.socket, server._verbose);
}

/**
 * @brief  Responsible for handling the Close Auction request and consult the
 * database.
 * @param  &message: The adapter containing the raw received message.
 * @param  &server: Instance of the server.
 * @param  &address: The address to where the message should go.
 * @throws UserDoesNotExist if a non-existant user attempts to perform the
 * request.
 * @throws IncorrectPassword if the password does not match up with the user's
 * supposed one.
 * @throws UserNotLoggedIn if a logged out user attempts to perform the request.
 * @throws AuctionNotFound if attempting to close a non-existant auction.
 * @throws AuctionNotOwnedByUser if a user attempts to close an auction that's
 * not theirs.
 * @throws AuctionAlreadyClosed if attempting to close an already closed
 * auction.
 * @throws InvalidMessageException if the message is wrongly formatted or
 * conatins invalid contents.
 * @retval None
 */
void CloseAuctionRequest::handle(MessageAdapter &message, Server &server,
                                 Address &address) {
	ClientCloseAuction message_in;
	ServerCloseAuction message_out;

	try {
		message_in.readMessage(message);
		if (server._verbose) {
			printAddressIncomingRequest(address);
			printInCloseAuctionRequest(message_in);
		}

		std::string user_id = convert_user_id_to_str(message_in.user_id);
		std::string auction_id =
			convert_auction_id_to_str(message_in.auction_id);

		// Access database
		int res = server._database.CloseAuction(auction_id, user_id,
		                                        message_in.password);
		if (res != DB_CLOSE_NOK) {
			message_out.status = ServerCloseAuction::status::OK;
		}

	} catch (UserDoesNotExist &e) {
		message_out.status = ServerCloseAuction::status::NOK;
	} catch (IncorrectPassword &e) {
		message_out.status = ServerCloseAuction::status::NOK;
	} catch (UserNotLoggedIn &e) {
		message_out.status = ServerCloseAuction::status::NLG;
	} catch (AuctionNotFound &e) {
		message_out.status = ServerCloseAuction::status::EAU;
	} catch (AuctionNotOwnedByUser &e) {
		message_out.status = ServerCloseAuction::status::EOW;
	} catch (AuctionAlreadyClosed &e) {
		message_out.status = ServerCloseAuction::status::END;
	} catch (InvalidMessageException &e) {
		message_out.status = ServerCloseAuction::status::ERR;
	} catch (std::exception &e) {
		printError("Failed to handle 'CLOSE AUCTION' request." +
		           std::string(e.what()));
		return;
	}

	send_tcp_message(message_out, address.socket, server._verbose);
}

/**
 * @brief  Responsible for handling the Show Asset request and consult the
 * database.
 * @param  &message: The adapter containing the raw received message.
 * @param  &server: Instance of the server.
 * @param  &address: The address to where the message should go.
 * @throws AssetDoesNotExist if attempting to obtain an non-existant auction.
 * @throws InvalidMessageException if the message is wrongly formatted or
 * conatins invalid contents.
 * @retval None
 */
void ShowAssetRequest::handle(MessageAdapter &message, Server &server,
                              Address &address) {
	ClientShowAsset message_in;
	ServerShowAsset message_out;

	try {
		message_in.readMessage(message);
		if (server._verbose) {
			printAddressIncomingRequest(address);
			printInShowAssetRequest(message_in);
		}

		std::string aid_str = convert_auction_id_to_str(message_in.auction_id);

		// Access database
		AssetInfo ast_info = server._database.ShowAsset(aid_str);

		message_out.status = ServerShowAsset::status::OK;
		message_out.fname = ast_info.asset_fname;
		message_out.fsize = ast_info.fsize;
		message_out.fdata = ast_info.fdata;

	} catch (AssetDoesNotExist &e) {
		message_out.status = ServerShowAsset::status::NOK;
	} catch (InvalidMessageException &e) {
		message_out.status = ServerShowAsset::status::ERR;
	} catch (std::exception &e) {
		printError("Failed to handle 'SHOW ASSET' request." +
		           std::string(e.what()));
		return;
	}

	send_tcp_message(message_out, address.socket, server._verbose);
}

/**
 * @brief  Responsible for handling the Bid request and consult the database.
 * @param  &message: The adapter containing the raw received message.
 * @param  &server: Instance of the server.
 * @param  &address: The address to where the message should go.
 * @throws AuctionAlreadyClosed if attempting to close an already closed
 * auction.
 * @throws UserNotLoggedIn if a logged out user attempts to perform the request.
 * @throws AuctionNotFound if attempting to close a non-existant auction.
 * @throws LargerBidAlreadyExists if attempting to place a bid that's too small.
 * @throws BidOnSelf if attempting to bid on an auction the user created.
 * @throws InvalidMessageException if the message is wrongly formatted or
 * conatins invalid contents.
 *
 * @retval None
 */
void BidRequest::handle(MessageAdapter &message, Server &server,
                        Address &address) {
	ClientBid message_in;
	ServerBid message_out;

	try {
		message_in.readMessage(message);
		if (server._verbose) {
			printAddressIncomingRequest(address);
			printInBidRequest(message_in);
		}

		std::string user_id = convert_user_id_to_str(message_in.user_id);
		std::string auction_id =
			convert_auction_id_to_str(message_in.auction_id);
		std::string bid_value = std::to_string(message_in.value);

		// Access database
		int res = server._database.Bid(user_id, message_in.password, auction_id,
		                               bid_value);
		if (res == DB_BID_NOK) {
			message_out.status = ServerBid::status::NOK;
		} else {
			message_out.status = ServerBid::status::ACC;
		}

	} catch (AuctionAlreadyClosed &e) {
		message_out.status = ServerBid::status::NOK;
	} catch (UserNotLoggedIn &e) {
		message_out.status = ServerBid::status::NLG;
	} catch (AuctionNotFound &e) {
		message_out.status = ServerBid::status::NOK;
	} catch (LargerBidAlreadyExists &e) {
		message_out.status = ServerBid::status::REF;
	} catch (BidOnSelf &e) {
		message_out.status = ServerBid::status::ILG;
	} catch (InvalidMessageException &e) {
		message_out.status = ServerBid::status::ERR;
	} catch (std::exception &e) {
		printError("Failed to handle 'BID' request." + std::string(e.what()));
		return;
	}

	send_tcp_message(message_out, address.socket, server._verbose);
}

/**
 * @brief  Respponsible for handling a wrong UDP request.
 * @param  &message: The adapter containing the raw received message.
 * @param  &server: Instance of the server.
 * @param  &address: The address to where the message should go.
 * @retval None
 */
void WrongRequestUDP::handle(MessageAdapter &message, Server &server,
                             Address &address) {
	(void) message;

	ServerError message_out;

	send_udp_message(message_out, address.socket,
	                 (struct sockaddr *) &address.addr, address.size,
	                 server._verbose);
}

/**
 * @brief  Responsible for handling a wrong TCP request.
 * @param  &message: The adapter containing the raw received message.
 * @param  &server: Instance of the server.
 * @param  &address: The address to where the message should go.
 * @retval None
 */
void WrongRequestTCP::handle(MessageAdapter &message, Server &server,
                             Address &address) {
	(void) message;

	ServerError message_out;

	send_tcp_message(message_out, address.socket, server._verbose);
}