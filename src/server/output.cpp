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
// | Utilities						 |
// -----------------------------------

std::string hidePassword(std::string password) {
	std::string hidden_password = "";
	for (int i = 0; i < password.length(); i++) {
		hidden_password += "*";
	};
	return hidden_password;
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

void printInLoginRequest(ClientLoginUser request) {
	std::string message = "\tIncoming 'LOGIN':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n\t<- Password: ";
	message += hidePassword(request.password);
	printInfo(message);
}

void printInLogoutRequest(ClientLogout request) {
	std::string message = "\tIncoming 'LOGOUT':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n\t<- Password: ";
	message += hidePassword(request.password);
	printInfo(message);
}

void printInUnregisterRequest(ClientUnregister request) {
	std::string message = "\tIncoming 'UNREGISTER':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n\t<- Password: ";
	message += hidePassword(request.password);
	printInfo(message);
}

void printInListAllRequest(ClientListAllAuctions request) {
	std::string message =
		"\tIncoming 'LIST ALL AUCTIONS':\n\t<- (no arguments) ";
	printInfo(message);
}

void printInListBiddedRequest(ClientListBiddedAuctions request) {
	std::string message =
		"\tIncoming 'LIST USER'S BIDDED AUCTIONS':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	printInfo(message);
}

void printInListStartedRequest(ClientListStartedAuctions request) {
	std::string message =
		"\tIncoming 'LIST USER'S STARTED AUCTIONS':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	printInfo(message);
}

void printInShowRecordRequest(ClientShowRecord request) {
	std::string message = "\tIncoming 'SHOW RECORD':\n\t<- Auction ID: ";
	message += convert_auction_id_to_str(request.auction_id);
	printInfo(message);
}

void printInOpenAuctionRequest(ClientOpenAuction request) {
	std::string message = "\tIncoming 'OPEN AUCTION':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n\t<- Password: ";
	message += hidePassword(request.password);
	message += "\n\t<- Auction Name: ";
	message += request.name;
	message += "\n\t<- Starting Value: ";
	message += std::to_string(request.start_value);
	message += "\n\t<- Max Time Active: ";
	message += std::to_string(request.timeactive);
	message += "\n\t<- Asset File Name: ";
	message += request.assetf_name;
	message += "\n\t<- Asset File Size: ";
	message += request.fsize / 1000000;
	message += " MB";
	message += "\n\t<- Asset File Data Length: ";
	message += request.fdata.length();
	printInfo(message);
}

void printInCloseAuctionRequest(ClientCloseAuction request) {
	std::string message = "\tIncoming 'CLOSE AUCTION':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n\t<- Password: ";
	message += hidePassword(request.password);
	message += "\n\t<- Auction ID: ";
	message += convert_auction_id_to_str(request.auction_id);
	printInfo(message);
}

void printInShowAssetRequest(ClientShowAsset request) {
	std::string message = "TODO";
	printInfo(message);
}

void printInBidRequest(ClientBid request) {
	std::string message = "TODO";
	printInfo(message);
}