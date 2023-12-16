#include "output.hpp"

#include <arpa/inet.h>

#include <iostream>

#include "server.hpp"
#include "shared/protocol.hpp"

// -----------------------------------
// | BASIC PRINTS					 |
// -----------------------------------

/**
 * @brief  Prints an error message.
 * @param  message: The type of message
 * @retval None
 */
void printError(std::string message) {
	std::cout << "[ERROR] " << message << std::endl;
}

/**
 * @brief  Prints an info message.
 * @param  message: The type of message
 * @param  tab_level: ammount of tabs before the message. Level of message.
 * @retval None
 */
void printInfo(std::string message, int tab_level) {
	std::string prefix;
	for (int i = 0; i < tab_level; i++) {
		prefix += "\t";
	}
	std::cout << prefix << "[INFO] " << message << std::endl;
}

/**
 * @brief  Prints an request message.
 * @param  message: The type of message
 * @retval None
 */
void printRequest(std::string message) {
	std::cout << "[REQUEST] " << message << std::endl;
}

// -----------------------------------
// | Utilities						 |
// -----------------------------------

/**
 * @brief  Censors the password.
 * @param  password: The password to censor.
 * @retval The censored password.
 */
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

/**
 * @brief  Prints an incoming request from a given address and port.
 * @param  &addr_from: The address given.
 * @retval None
 */
void printAddressIncomingRequest(Address &addr_from) {
	char addr_str[INET_ADDRSTRLEN + 1] = {0};
	inet_ntop(AF_INET, &addr_from.addr.sin_addr, addr_str, INET_ADDRSTRLEN);
	std::string message = "Receiving incoming UDP request from ";
	message += addr_str;
	message += ":";
	message += std::to_string(ntohs(addr_from.addr.sin_port));
	message += "\n";
	printInfo(message, 0);
}

/**
 * @brief  Prints the login request with the given user id and password.
 * @param  request: Struct containing the info.
 * @retval None
 */
void printInLoginRequest(ClientLoginUser request) {
	std::string message = "\tIncoming 'LOGIN':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n\t<- Password: ";
	message += hidePassword(request.password);
	message += "\n";
	printInfo(message, 1);
}

/**
 * @brief  Prints the logout request with the given user id and password.
 * @param  request: Struct containing the info.
 * @retval None
 */
void printInLogoutRequest(ClientLogout request) {
	std::string message = "\tIncoming 'LOGOUT':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n\t<- Password: ";
	message += hidePassword(request.password);
	message += "\n";
	printInfo(message, 1);
}

/**
 * @brief  Prints the unregister request with the given user id and password.
 * @param  request: Struct containing the info.
 * @retval None
 */
void printInUnregisterRequest(ClientUnregister request) {
	std::string message = "\tIncoming 'UNREGISTER':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n\t<- Password: ";
	message += hidePassword(request.password);
	message += "\n";
	printInfo(message, 1);
}

/**
 * @brief  Prints the list all auctions request.
 * @param  request: Struct containing the info.
 * @retval None
 */
void printInListAllRequest(ClientListAllAuctions request) {
	(void) request;
	std::string message =
		"\tIncoming 'LIST ALL AUCTIONS':\n\t<- (no arguments) ";
	message += "\n";
	printInfo(message, 1);
}

/**
 * @brief  Prints the list bidded auctions request with the given user id.
 * @param  request: Struct containing the info.
 * @retval None
 */
void printInListBiddedRequest(ClientListBiddedAuctions request) {
	std::string message =
		"\tIncoming 'LIST USER'S BIDDED AUCTIONS':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n";
	printInfo(message, 1);
}

/**
 * @brief  Prints the list started auctions request with the given user id.
 * @param  request: Struct containing the info.
 * @retval None
 */
void printInListStartedRequest(ClientListStartedAuctions request) {
	std::string message =
		"\tIncoming 'LIST USER'S STARTED AUCTIONS':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n";
	printInfo(message, 1);
}

/**
 * @brief  Prints the show record request with the given auction id.
 * @param  request: Struct containing the info.
 * @retval None
 */
void printInShowRecordRequest(ClientShowRecord request) {
	std::string message = "\tIncoming 'SHOW RECORD':\n\t<- Auction ID: ";
	message += convert_auction_id_to_str(request.auction_id);
	message += "\n";
	printInfo(message, 1);
}

/**
 * @brief  Prints the open auction request with the given user id, password,
 * * auction name, starting value, time active, name and size of the asset file
 * and the length of its data.
 * @param  request: Struct containing the info.
 * @retval None
 */
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

/**
 * @brief  Prints the close auction request with the given user id, password and
 * auction id.
 * @param  request: Struct containing the info.
 * @retval None
 */
void printInCloseAuctionRequest(ClientCloseAuction request) {
	std::string message = "\tIncoming 'CLOSE AUCTION':\n\t<- User ID: ";
	message += convert_user_id_to_str(request.user_id);
	message += "\n\t<- Password: ";
	message += hidePassword(request.password);
	message += "\n\t<- Auction ID: ";
	message += convert_auction_id_to_str(request.auction_id);
	message += "\n";
	printInfo(message, 1);
}

/**
 * @brief  Prints the show asset request with the given auction id.
 * @param  request: Struct containing the info.
 * @retval None
 */
void printInShowAssetRequest(ClientShowAsset request) {
	std::string message = "\tIncoming 'SHOW ASSET':\n\t<- Auction ID: ";
	message += convert_auction_id_to_str(request.auction_id);
	message += "\n";
	printInfo(message, 1);
}

/**
 * @brief  Prints the bid request with the given user id, password, auction id
 * and bid value.
 * @param  request: Struct containing the info.
 * @retval None
 */
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
	printInfo(message, 1);
}