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

	int length = static_cast<int>(password.length());

	for (int i = 0; i < length; i++) {
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
	message += "\n";
	printInfo(message);
}

void printInLoginRequest(ClientLoginUser request) {
	std::string message = "\tIncoming 'LOGIN':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n\t<- Password: ";
	message += hidePassword(request.password);
	message += "\n";
	printInfo(message);
}

void printInLogoutRequest(ClientLogout request) {
	std::string message = "\tIncoming 'LOGOUT':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n\t<- Password: ";
	message += hidePassword(request.password);
	message += "\n";
	printInfo(message);
}

void printInUnregisterRequest(ClientUnregister request) {
	std::string message = "\tIncoming 'UNREGISTER':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n\t<- Password: ";
	message += hidePassword(request.password);
	message += "\n";
	printInfo(message);
}

void printInListAllRequest(ClientListAllAuctions request) {
	(void) request;
	std::string message =
		"\tIncoming 'LIST ALL AUCTIONS':\n\t<- (no arguments) ";
	message += "\n";
	printInfo(message);
}

void printInListBiddedRequest(ClientListBiddedAuctions request) {
	std::string message =
		"\tIncoming 'LIST USER'S BIDDED AUCTIONS':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n";
	printInfo(message);
}

void printInListStartedRequest(ClientListStartedAuctions request) {
	std::string message =
		"\tIncoming 'LIST USER'S STARTED AUCTIONS':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n";
	printInfo(message);
}

void printInShowRecordRequest(ClientShowRecord request) {
	std::string message = "\tIncoming 'SHOW RECORD':\n\t<- Auction ID: ";
	message += convert_auction_id_to_str(request.auction_id);
	message += "\n";
	printInfo(message);
}

void printInOpenAuctionRequest(ClientOpenAuction request) {
	std::cout << "[INFO]\tIncoming 'OPEN AUCTION':\n\t<- User ID: "
			  << convert_user_id_to_str(request.user_id)
			  << "\n\t<- Password: " << hidePassword(request.password)
			  << "\n\t<- Auction Name: " << request.name
			  << "\n\t<- Starting Value: "
			  << std::to_string(request.start_value)
			  << "\n\t<- Max Time Active: "
			  << std::to_string(request.timeactive)
			  << "\n\t<- Asset File Name: " << request.assetf_name
			  << std::setprecision(3) << std::fixed
			  << "\n\t<- Asset File Size: " << (float) request.Fsize / (1000000)
			  << " MB"
			  << "\n\t<- Asset File Data Length: "
			  << std::to_string(request.fdata.length()) << " bytes. \n"
			  << std::endl;
}

void printInCloseAuctionRequest(ClientCloseAuction request) {
	std::string message = "\tIncoming 'CLOSE AUCTION':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n\t<- Password: ";
	message += hidePassword(request.password);
	message += "\n\t<- Auction ID: ";
	message += convert_auction_id_to_str(request.auction_id);
	message += "\n";
	printInfo(message);
}

void printInShowAssetRequest(ClientShowAsset request) {
	std::string message = "\tIncoming 'SHOW ASSET':\n\t<- Auction ID: ";
	message += convert_auction_id_to_str(request.auction_id);
	message += "\n";
	printInfo(message);
}

void printInBidRequest(ClientBid request) {
	std::string message = "\tIncoming 'BID':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n\t<- Password: ";
	message += hidePassword(request.password);
	message += "\n\t<- Auction ID: ";
	message += convert_auction_id_to_str(request.auction_id);
	message += "\n\t<- Value: ";
	message += std::to_string(request.value);
	message += "\n";
	printInfo(message);
}