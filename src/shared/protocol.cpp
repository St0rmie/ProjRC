#include "protocol.hpp"

/**
 * @file protocol.cpp
 * @brief This file contains the implementation of the Protocol used in the
 * communication between the Auction Server and the user Client.
 */

// -----------------------------------
// | Reading functions				 |
// -----------------------------------

/**
 * @brief  Reads a character from the buffer.
 * @param  &buffer: adapter
 * @throws InvalidMessageException
 * @retval char
 */
char ProtocolMessage::readChar(MessageAdapter &buffer) {
	char c;
	c = buffer.get();
	if (!buffer.good()) {
		throw InvalidMessageException();
	}
	return c;
}

/**
 * @brief  Reads a character from the buffer and compares it to a given char,
 * expecting them to be equal.
 * @param  &buffer: adapter
 * @throws InvalidMessageException
 * @retval char
 */
void ProtocolMessage::readChar(MessageAdapter &buffer, char c) {
	char read = readChar(buffer);
	if (read != c) {
		throw InvalidMessageException();
	}
}

/**
 * @brief  Reads a character from the buffer and compares it to a given char
 * evaluating if they are equal.
 * @param  &buffer: adapter
 * @retval true if equal, false otherwise
 */
bool ProtocolMessage::readCharEqual(MessageAdapter &buffer, char c) {
	char read = readChar(buffer);
	if (read != c) {
		buffer.unget();
		return false;
	} else {
		return true;
	}
}

/**
 * @brief  Reads a messageID from the buffer and compares it to a given protocol
 * code expecting them to be equal.
 * @param  &buffer: adapter
 * @throws UnexpectedMessageException
 * @throws ERRCodeMessageException
 * @retval None
 */
void ProtocolMessage::readMessageId(MessageAdapter &buffer,
                                    std::string protocol_code) {
	char current_char;
	std::string errcheck;
	size_t i = 0;
	while (protocol_code[i] != '\0') {
		current_char = readChar(buffer);
		errcheck += current_char;
		if (!buffer.good() || current_char != protocol_code[i]) {
			throw UnexpectedMessageException();
			if (errcheck == CODE_ERROR) {
				throw ERRCodeMessageException();
			}
		}
		++i;
	}
}

/**
 * @brief  Reads a space from the buffer.
 * @param  &buffer: adapter
 * @retval None
 */
void ProtocolMessage::readSpace(MessageAdapter &buffer) {
	readChar(buffer, ' ');
}

/**
 * @brief  Reads a delimiter ('\n') from the buffer.
 * @param  &buffer: adapter
 * @retval None
 */
void ProtocolMessage::readDelimiter(MessageAdapter &buffer) {
	readChar(buffer, '\n');
}

/**
 * @brief  Reads a string of a maximum size from the buffer. The function will
 * stop reading when it finds a space or a delimiter, even if it is below the
 * max length.
 * @param  &buffer: adapter
 * @retval string read
 */
std::string ProtocolMessage::readString(MessageAdapter &buffer,
                                        uint32_t max_len) {
	std::string str;
	for (uint32_t i = 0; i < max_len; i++) {
		char c = (char) buffer.get();
		if (!buffer.good()) {
			throw InvalidMessageException();
		}
		if (c == ' ' || c == '\n') {
			// Stops reading
			buffer.unget();
			break;
		}
		str += c;
	}

	return str;
}

/**
 * @brief  Reads a userid from the buffer.
 * @param  &buffer: adapter
 * @retval (uint32_t) user_id
 */
uint32_t ProtocolMessage::readUserId(MessageAdapter &buffer) {
	std::string id_str = readString(buffer, USER_ID_SIZE);
	return convert_user_id(id_str);
}

/**
 * @brief  Reads a auction id from the buffer.
 * @param  &buffer: adapter
 * @retval (uint32_t) auction id
 */
uint32_t ProtocolMessage::readAuctionId(MessageAdapter &buffer) {
	std::string id_str = readString(buffer, AUCTION_ID_SIZE);
	return convert_auction_id(id_str);
}

/**
 * @brief  Reads a auction value from the buffer.
 * @param  &buffer: adapter
 * @retval (uint32_t) auction value
 */
uint32_t ProtocolMessage::readAuctionValue(MessageAdapter &buffer) {
	std::string value_str = readString(buffer, MAX_AUCTION_VALUE_SIZE);
	return convert_auction_value(value_str);
}

/**
 * @brief  Reads a password from the buffer.
 * @param  &buffer: adapter
 * @retval (string) password
 */
std::string ProtocolMessage::readPassword(MessageAdapter &buffer) {
	std::string password_str = readString(buffer, PASSWORD_SIZE);
	return convert_password(password_str);
}

/**
 * @brief  Reads a pair of auction id and state in a string format from the
 * buffer.
 * @param  &buffer: adapter
 * @retval (sting) 'auction_id state'
 */
std::string ProtocolMessage::readAuctionAndState(MessageAdapter &buffer) {
	if (checkIfOver(buffer) == true) {
		return "";
	}

	readSpace(buffer);
	std::string auction_str = readString(buffer, AUCTION_ID_SIZE);
	readSpace(buffer);
	std::string state_str = readString(buffer, 1);
	if (state_str == "1") {
		state_str = "ACTIVE";
	} else if (state_str == "0") {
		state_str = "OVER";
	}
	return "AID: " + auction_str + " --- STATUS: " + state_str;
}

/**
 * @brief  Tries to read a delimiter from the buffer to see if the message has
 * ended.
 * @param  &buffer: adapter
 * * @retval true if the delimiter was found, false otherwise
 */
bool ProtocolMessage::checkIfOver(MessageAdapter &buffer) {
	char c = '\n';
	if (readChar(buffer) == c) {
		buffer.unget();
		return true;
	} else {
		buffer.unget();
		return false;
	}
}

/**
 * @brief  Reads a date in format Datetime from the buffer.
 * @param  &buffer: adapter
 * @retval (Datetime) date and time
 */
Datetime ProtocolMessage::readDate(MessageAdapter &buffer) {
	Datetime date;
	date.year = readString(buffer, 4);
	readChar(buffer, '-');
	date.month = readString(buffer, 2);
	readChar(buffer, '-');
	date.day = readString(buffer, 2);
	readChar(buffer, ' ');
	date.hours = readString(buffer, 2);
	readChar(buffer, ':');
	date.minutes = readString(buffer, 2);
	readChar(buffer, ':');
	date.seconds = readString(buffer, 2);
	return date;
}

/**
 * @brief  Reads file data of specified size from the buffer.
 * @param  &buffer: adapter
 * @retval (string) file data
 */
std::string ProtocolMessage::readFile(MessageAdapter &buffer,
                                      uint32_t max_len) {
	if (max_len > MAX_FILE_SIZE) {
		throw FileException();
		return "";
	}

	std::string str;
	for (uint32_t i = 0; i < max_len; i++) {
		char c = (char) buffer.get();
		if (!buffer.good()) {
			throw InvalidMessageException();
		}
		str += c;
	}

	return str;
}

// -----------------------------------
// | Types of protocol messages		 |
// -----------------------------------

// ---------- LOGIN

/**
 * @brief  Serializes a message of a Login request made by the client.
 * @retval (stringstream) Serialized message
 */
std::stringstream ClientLoginUser::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " " << user_id << " " << password << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a Login request made by the client.
 * @retval None
 */
void ClientLoginUser::readMessage(MessageAdapter &buffer) {
	// Server already read the message ID
	readSpace(buffer);
	user_id = readUserId(buffer);
	readSpace(buffer);
	password = readPassword(buffer);
	readDelimiter(buffer);
}

/**
 * @brief  Serializes a message of a Login answer made by the server.
 * @retval (stringstream) Serialized message
 */
std::stringstream ServerLoginUser::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " ";
	if (status == ServerLoginUser::status::OK) {
		buffer << "OK";
	} else if (status == ServerLoginUser::status::NOK) {
		buffer << "NOK";
	} else if (status == ServerLoginUser::status::REG) {
		buffer << "REG";
	} else if (status == ServerLoginUser::status::ERR) {
		buffer << "ERR";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a Login answer made by the server.
 * @retval None
 */
void ServerLoginUser::readMessage(MessageAdapter &buffer) {
	readMessageId(buffer, ServerLoginUser::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, MAX_STATUS_SIZE);
	if (status_str == "OK") {
		status = OK;
	} else if (status_str == "NOK") {
		status = NOK;
	} else if (status_str == "REG") {
		status = REG;
	} else if (status_str == "ERR") {
		status = ERR;
	} else {
		throw InvalidMessageException();
	}
	readDelimiter(buffer);
}

// ---------- LOGOUT

/**
 * @brief  Serializes a message of a Logout request made by the client.
 * @retval (stringstream) Serialized message
 */
std::stringstream ClientLogout::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " " << user_id << " " << password << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a Logout request made by the client.
 * @retval None
 */
void ClientLogout::readMessage(MessageAdapter &buffer) {
	// Serverbound messages don't read their ID
	readSpace(buffer);
	user_id = readUserId(buffer);
	readSpace(buffer);
	password = readPassword(buffer);
	readDelimiter(buffer);
}

/**
 * @brief  Serializes a message of a Logout answer made by the server.
 * @retval (stringstream) Serialized message
 */
std::stringstream ServerLogout::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " ";
	if (status == ServerLogout::status::OK) {
		buffer << "OK";
	} else if (status == ServerLogout::status::NOK) {
		buffer << "NOK";
	} else if (status == ServerLogout::status::UNR) {
		buffer << "UNR";
	} else if (status == ServerLogout::status::ERR) {
		buffer << "ERR";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a Logout answer made by the server.
 * @retval None
 */
void ServerLogout::readMessage(MessageAdapter &buffer) {
	readMessageId(buffer, ServerLogout::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, MAX_STATUS_SIZE);
	if (status_str == "OK") {
		status = OK;
	} else if (status_str == "NOK") {
		status = NOK;
	} else if (status_str == "UNR") {
		status = UNR;
	} else if (status_str == "ERR") {
		status = ERR;
	} else {
		throw InvalidMessageException();
	}
	readDelimiter(buffer);
}

// ---------- UNREGISTER

/**
 * @brief  Serializes a message of a Unregister request made by the client.
 * @retval (stringstream) Serialized message
 */
std::stringstream ClientUnregister::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " " << user_id << " " << password << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a Unregister request made by the client.
 * @retval None
 */
void ClientUnregister::readMessage(MessageAdapter &buffer) {
	// Serverbound packets don't read their ID
	readSpace(buffer);
	user_id = readUserId(buffer);
	readSpace(buffer);
	password = readPassword(buffer);
	readDelimiter(buffer);
}

/**
 * @brief  Serializes a message of a Unregister answer made by the server.
 * @retval (stringstream) Serialized message
 */
std::stringstream ServerUnregister::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " ";
	if (status == ServerUnregister::status::OK) {
		buffer << "OK";
	} else if (status == ServerUnregister::status::NOK) {
		buffer << "NOK";
	} else if (status == ServerUnregister::status::UNR) {
		buffer << "UNR";
	} else if (status == ServerUnregister::status::ERR) {
		buffer << "ERR";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a Unregister answer made by the server.
 * @retval None
 */
void ServerUnregister::readMessage(MessageAdapter &buffer) {
	readMessageId(buffer, ServerUnregister::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, MAX_STATUS_SIZE);
	if (status_str == "OK") {
		status = OK;
	} else if (status_str == "NOK") {
		status = NOK;
	} else if (status_str == "UNR") {
		status = UNR;
	} else if (status_str == "ERR") {
		status = ERR;
	} else {
		throw InvalidMessageException();
	}
	readDelimiter(buffer);
}

// ---------- LIST MYAUCTIONS

/**
 * @brief  Serializes a message of a List My Auctions request made by the
 * client.
 * @retval (stringstream) Serialized message
 */
std::stringstream ClientListStartedAuctions::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " " << user_id << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a List My Auctions request made by the client.
 * @retval None
 */
void ClientListStartedAuctions::readMessage(MessageAdapter &buffer) {
	// Serverbound packets don't read their ID
	readSpace(buffer);
	user_id = readUserId(buffer);
	readDelimiter(buffer);
}

/**
 * @brief  Serializes a message of a List My Auctions answer made by the
 * server.
 * @retval (stringstream) Serialized message
 */
std::stringstream ServerListStartedAuctions::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " ";
	if (status == ServerListStartedAuctions::status::OK) {
		buffer << "OK";
		for (std::string auction : auctions) {
			buffer << " " << auction;
		}
	} else if (status == ServerListStartedAuctions::status::NOK) {
		buffer << "NOK";
	} else if (status == ServerListStartedAuctions::status::NLG) {
		buffer << "NLG";
	} else if (status == ServerListStartedAuctions::status::ERR) {
		buffer << "ERR";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a List My Auctions answer made by the server.
 * @retval None
 */
void ServerListStartedAuctions::readMessage(MessageAdapter &buffer) {
	readMessageId(buffer, ServerListStartedAuctions::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, MAX_STATUS_SIZE);
	if (status_str == "OK") {
		status = OK;
		std::string auc;
		while ((auc = readAuctionAndState(buffer)).length() > 0) {
			ServerListStartedAuctions::auctions.push_back(auc);
		}
		if (auctions.size() == 0) {
			status = ERR;
		}
	} else if (status_str == "NOK") {
		status = NOK;
	} else if (status_str == "NLG") {
		status = NLG;
	} else if (status_str == "ERR") {
		status = ERR;
	} else {
		throw InvalidMessageException();
	}
	readDelimiter(buffer);
}

// ---------- LIST MYBIDDEDAUCTIONS

/**
 * @brief  Serializes a message of a List My Bids request made by the client.
 * @retval (stringstream) Serialized message
 */
std::stringstream ClientListBiddedAuctions::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " " << user_id << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a List My Bids request made by the client.
 * @retval None
 */
void ClientListBiddedAuctions::readMessage(MessageAdapter &buffer) {
	// Serverbound packets don't read their ID
	readSpace(buffer);
	user_id = readUserId(buffer);
	readDelimiter(buffer);
}

/**
 * @brief  Serializes a message of a List My Bids answer made by the server.
 * @retval (stringstream) Serialized message
 */
std::stringstream ServerListBiddedAuctions::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " ";
	if (status == ServerListBiddedAuctions::status::OK) {
		buffer << "OK";
		for (std::string auction : auctions) {
			buffer << " " << auction;
		}
	} else if (status == ServerListBiddedAuctions::status::NOK) {
		buffer << "NOK";
	} else if (status == ServerListBiddedAuctions::status::NLG) {
		buffer << "NLG";
	} else if (status == ServerListBiddedAuctions::status::ERR) {
		buffer << "ERR";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a List My Bids answer made by the server.
 * @retval None
 */
void ServerListBiddedAuctions::readMessage(MessageAdapter &buffer) {
	readMessageId(buffer, ServerListBiddedAuctions::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, MAX_STATUS_SIZE);
	if (status_str == "OK") {
		status = OK;
		std::string auc;
		while ((auc = readAuctionAndState(buffer)).length() > 0) {
			ServerListBiddedAuctions::auctions.push_back(auc);
		}
		if (auctions.size() == 0) {
			status = ERR;
		}
	} else if (status_str == "NOK") {
		status = NOK;
	} else if (status_str == "NLG") {
		status = NLG;
	} else if (status_str == "ERR") {
		status = ERR;
	} else {
		throw InvalidMessageException();
	}
	readDelimiter(buffer);
}

// ---------- LIST ALL AUCTIONS

/**
 * @brief  Serializes a message of a List All Auctions request made by the
 * client.
 * @retval (stringstream) Serialized message
 */
std::stringstream ClientListAllAuctions::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a List My Auctions request made by the client.
 * @retval None
 */
void ClientListAllAuctions::readMessage(MessageAdapter &buffer) {
	// Serverbound packets don't read their ID
	readDelimiter(buffer);
}

/**
 * @brief  Serializes a message of a List All Auctions answer made by the
 * server.
 * @retval (stringstream) Serialized message
 */
std::stringstream ServerListAllAuctions::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " ";
	if (status == ServerListAllAuctions::status::OK) {
		buffer << "OK";
		for (std::string auction : auctions) {
			buffer << " " << auction;
		}
	} else if (status == ServerListAllAuctions::status::NOK) {
		buffer << "NOK";
	} else if (status == ServerListAllAuctions::status::ERR) {
		buffer << "ERR";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a List All Auctions answer made by the server.
 * @retval None
 */
void ServerListAllAuctions::readMessage(MessageAdapter &buffer) {
	readMessageId(buffer, ServerListAllAuctions::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, MAX_STATUS_SIZE);
	if (status_str == "OK") {
		status = OK;
		std::string auc;
		while ((auc = readAuctionAndState(buffer)).length() > 0) {
			ServerListAllAuctions::auctions.push_back(auc);
		}
		if (auctions.size() == 0) {
			status = ERR;
		}
	} else if (status_str == "NOK") {
		status = NOK;
	} else if (status_str == "ERR") {
		status = ERR;
	} else {
		throw InvalidMessageException();
	}
	readDelimiter(buffer);
}

// ---------- SHOW RECORD

/**
 * @brief  Serializes a message of a Show Record request made by the client.
 * @retval (stringstream) Serialized message
 */
std::stringstream ClientShowRecord::buildMessage() {
	std::stringstream buffer;
	char aid[4];
	sprintf(aid, "%03d", auction_id);
	buffer << protocol_code << " " << aid << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a Show Record request made by the client.
 * @retval None
 */
void ClientShowRecord::readMessage(MessageAdapter &buffer) {
	readSpace(buffer);
	auction_id = readAuctionId(buffer);
	readDelimiter(buffer);
}

/**
 * @brief  Serializes a message of a Show Record answer made by the server.
 * @retval (stringstream) Serialized message
 */
std::stringstream ServerShowRecord::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " ";
	if (status == ServerShowRecord::status::OK) {
		buffer << "OK ";
		buffer << host_UID;
		buffer << " " << auction_name;
		buffer << " " << asset_fname;
		buffer << " " << start_value;
		buffer << " " << convert_date_to_str(start_date_time);
		buffer << " " << timeactive;
		for (Bid bid : bids) {
			buffer << " B " << bid.bidder_UID;
			buffer << " " << bid.bid_value;
			buffer << " " << convert_date_to_str(bid.bid_date_time);
			buffer << " " << bid.bid_sec_time;
		}
		if (end_sec_time > 0) {
			buffer << " E " << convert_date_to_str(end_date_time);
			buffer << " " << end_sec_time;
		}
	} else if (status == ServerShowRecord::status::NOK) {
		buffer << "NOK";
	} else if (status == ServerShowRecord::status::ERR) {
		buffer << "ERR";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a Show Record answer made by the server.
 * @retval None
 */
void ServerShowRecord::readMessage(MessageAdapter &buffer) {
	bool skip;
	readMessageId(buffer, ServerShowRecord::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, MAX_STATUS_SIZE);
	if (status_str == "OK") {
		status = OK;
		readSpace(buffer);
		host_UID = readUserId(buffer);
		readSpace(buffer);
		auction_name = readString(buffer, MAX_AUCTION_NAME_SIZE);
		readSpace(buffer);
		asset_fname = readString(buffer, MAX_FILENAME_SIZE);
		readSpace(buffer);
		start_value = readAuctionValue(buffer);
		readSpace(buffer);
		start_date_time = readDate(buffer);
		readSpace(buffer);
		timeactive = static_cast<uint32_t>(
			stol(readString(buffer, MAX_TIMEACTIVE_SIZE)));
		if (readCharEqual(buffer, ' ')) {
			while (readCharEqual(buffer, 'B')) {
				Bid bid;
				readSpace(buffer);
				bid.bidder_UID = readUserId(buffer);
				readSpace(buffer);
				bid.bid_value = static_cast<uint32_t>(
					stol(readString(buffer, MAX_AUCTION_VALUE_SIZE)));
				readSpace(buffer);
				bid.bid_date_time = readDate(buffer);
				readSpace(buffer);
				bid.bid_sec_time = static_cast<uint32_t>(
					stol(readString(buffer, MAX_LENGTH_TIMEACTIVE)));
				bids.push_back(bid);
				skip = readCharEqual(buffer, ' ');
				if (!skip) {
					break;
				}
			};
			if (readCharEqual(buffer, 'E')) {
				readSpace(buffer);
				end_date_time = readDate(buffer);
				readSpace(buffer);
				end_sec_time = static_cast<uint32_t>(
					stol(readString(buffer, MAX_LENGTH_TIMEACTIVE)));
				readDelimiter(buffer);
			}
		}
	} else if (status_str == "NOK") {
		status = NOK;
		readDelimiter(buffer);
	} else if (status_str == "ERR") {
		status = ERR;
		readDelimiter(buffer);
	} else {
		throw InvalidMessageException();
	}
}

// ---------- OPEN AUCTION

/**
 * @brief  Serializes a message of a Open Auction request made by the client.
 * @retval (stringstream) Serialized message
 */
std::stringstream ClientOpenAuction::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " " << user_id << " " << password << " " << name
		   << " " << start_value << " " << timeactive << " " << assetf_name
		   << " " << Fsize << " " << fdata << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a Open Auction request made by the client.
 * @retval None
 */
void ClientOpenAuction::readMessage(MessageAdapter &buffer) {
	readSpace(buffer);
	user_id = readUserId(buffer);
	readSpace(buffer);
	password = readPassword(buffer);
	readSpace(buffer);
	name = readString(buffer, MAX_AUCTION_NAME_SIZE);
	readSpace(buffer);
	start_value = readAuctionValue(buffer);
	readSpace(buffer);
	timeactive =
		static_cast<uint32_t>(stol(readString(buffer, MAX_LENGTH_TIMEACTIVE)));
	readSpace(buffer);
	assetf_name = readString(buffer, MAX_FILENAME_SIZE);
	readSpace(buffer);
	Fsize = (size_t) stol(readString(buffer, MAX_FILE_SIZE_LENGTH));
	readSpace(buffer);
	fdata = readFile(buffer, static_cast<uint32_t>(Fsize));
	readDelimiter(buffer);
}

/**
 * @brief  Serializes a message of a Open Auction answer made by the server.
 * @retval (stringstream) Serialized message
 */
std::stringstream ServerOpenAuction::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " ";
	if (status == ServerOpenAuction::status::OK) {
		char aid[4];
		sprintf(aid, "%03d", auction_id);
		buffer << "OK " << aid;
	} else if (status == ServerOpenAuction::status::NOK) {
		buffer << "NOK";
	} else if (status == ServerOpenAuction::status::ERR) {
		buffer << "ERR";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a Open Auction answer made by the server.
 * @retval None
 */
void ServerOpenAuction::readMessage(MessageAdapter &buffer) {
	readMessageId(buffer, ServerOpenAuction::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, MAX_STATUS_SIZE);
	if (status_str == "OK") {
		status = OK;
		readSpace(buffer);
		auction_id = readAuctionId(buffer);
		readDelimiter(buffer);
	} else if (status_str == "NOK") {
		status = NOK;
		readDelimiter(buffer);
	} else if (status_str == "ERR") {
		status = ERR;
		readDelimiter(buffer);
	} else {
		throw InvalidMessageException();
	}
}

// ---------- CLOSE AUCTION

/**
 * @brief  Serializes a message of a Close Auction request made by the client.
 * @retval (stringstream) Serialized message
 */
std::stringstream ClientCloseAuction::buildMessage() {
	std::stringstream buffer;
	char aid[4];
	sprintf(aid, "%03d", auction_id);
	buffer << protocol_code << " " << user_id << " " << password << " " << aid
		   << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a Close Auction request made by the client.
 * @retval None
 */
void ClientCloseAuction::readMessage(MessageAdapter &buffer) {
	readSpace(buffer);
	user_id = readUserId(buffer);
	readSpace(buffer);
	password = readPassword(buffer);
	readSpace(buffer);
	auction_id = readAuctionId(buffer);
	readDelimiter(buffer);
}

/**
 * @brief  Serializes a message of a Close Auction answer made by the server.
 * @retval (stringstream) Serialized message
 */
std::stringstream ServerCloseAuction::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " ";
	if (status == ServerCloseAuction::status::OK) {
		buffer << "OK";
	} else if (status == ServerCloseAuction::status::NLG) {
		buffer << "NLG";
	} else if (status == ServerCloseAuction::status::EAU) {
		buffer << "EAU";
	} else if (status == ServerCloseAuction::status::EOW) {
		buffer << "EOW";
	} else if (status == ServerCloseAuction::status::END) {
		buffer << "END";
	} else if (status == ServerCloseAuction::status::NOK) {
		buffer << "NOK";
	} else if (status == ServerCloseAuction::status::ERR) {
		buffer << "ERR";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a Close Auction answer made by the server.
 * @retval None
 */
void ServerCloseAuction::readMessage(MessageAdapter &buffer) {
	readMessageId(buffer, ServerCloseAuction::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, MAX_STATUS_SIZE);
	readDelimiter(buffer);
	if (status_str == "OK") {
		status = OK;
	} else if (status_str == "NLG") {
		status = NLG;
	} else if (status_str == "EAU") {
		status = EAU;
	} else if (status_str == "EOW") {
		status = EOW;
	} else if (status_str == "END") {
		status = END;
	} else if (status_str == "NOK") {
		status = NOK;
	} else if (status_str == "ERR") {
		status = ERR;
	} else {
		throw InvalidMessageException();
	}
}

// ---------- SHOW ASSET

/**
 * @brief  Serializes a message of a Show Asset request made by the client.
 * @retval (stringstream) Serialized message
 */
std::stringstream ClientShowAsset::buildMessage() {
	std::stringstream buffer;
	char aid[4];
	sprintf(aid, "%03d", auction_id);
	buffer << protocol_code << " " << aid << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a Show Asset request made by the client.
 * @retval None
 */
void ClientShowAsset::readMessage(MessageAdapter &buffer) {
	readSpace(buffer);
	auction_id = readAuctionId(buffer);
	readDelimiter(buffer);
}

/**
 * @brief  Serializes a message of a Show Asset answer made by the server.
 * @retval (stringstream) Serialized message
 */
std::stringstream ServerShowAsset::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " ";
	if (status == ServerShowAsset::status::OK) {
		buffer << "OK " << fname << " " << fsize << " " << fdata;
	} else if (status == ServerShowAsset::status::NOK) {
		buffer << "NOK";
	} else if (status == ServerShowAsset::status::ERR) {
		buffer << "ERR";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a Show Asset answer made by the server.
 * @retval None
 */
void ServerShowAsset::readMessage(MessageAdapter &buffer) {
	readMessageId(buffer, ServerShowAsset::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, MAX_STATUS_SIZE);
	if (status_str == "OK") {
		status = OK;
		readSpace(buffer);
		fname = readString(buffer, MAX_FILENAME_SIZE);
		readSpace(buffer);
		fsize =
			static_cast<size_t>(stol(readString(buffer, MAX_FILE_SIZE_LENGTH)));
		readSpace(buffer);
		fdata = readFile(buffer, static_cast<uint32_t>(fsize));
		readDelimiter(buffer);
	} else if (status_str == "NOK") {
		status = NOK;
		readDelimiter(buffer);
	} else if (status_str == "ERR") {
		status = ERR;
		readDelimiter(buffer);
	} else {
		throw InvalidMessageException();
	}
}

// ---------- BID

/**
 * @brief  Serializes a message of a Bid request made by the client.
 * @retval (stringstream) Serialized message
 */
std::stringstream ClientBid::buildMessage() {
	std::stringstream buffer;
	char aid[4];
	sprintf(aid, "%03d", auction_id);
	buffer << protocol_code << " " << user_id << " " << password << " " << aid
		   << " " << value << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a Bid request made by the client.
 * @retval None
 */
void ClientBid::readMessage(MessageAdapter &buffer) {
	readSpace(buffer);
	user_id = readUserId(buffer);
	readSpace(buffer);
	password = readPassword(buffer);
	readSpace(buffer);
	auction_id = readAuctionId(buffer);
	readSpace(buffer);
	value = readAuctionValue(buffer);
	readDelimiter(buffer);
}

/**
 * @brief  Serializes a message of a Bid answer made by the server.
 * @retval (stringstream) Serialized message
 */
std::stringstream ServerBid::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " ";
	if (status == ServerBid::status::NOK) {
		buffer << "NOK";
	} else if (status == ServerBid::status::NLG) {
		buffer << "NLG";
	} else if (status == ServerBid::status::ACC) {
		buffer << "ACC";
	} else if (status == ServerBid::status::REF) {
		buffer << "REF";
	} else if (status == ServerBid::status::ILG) {
		buffer << "ILG";
	} else if (status == ServerBid::status::ERR) {
		buffer << "ERR";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a Bid answer made by the server.
 * @retval None
 */
void ServerBid::readMessage(MessageAdapter &buffer) {
	readMessageId(buffer, ServerBid::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, MAX_STATUS_SIZE);
	if (status_str == "NOK") {
		status = NOK;
		readDelimiter(buffer);
	} else if (status_str == "NLG") {
		status = NLG;
		readDelimiter(buffer);
	} else if (status_str == "ACC") {
		status = ACC;
		readDelimiter(buffer);
	} else if (status_str == "REF") {
		status = REF;
		readDelimiter(buffer);
	} else if (status_str == "ILG") {
		status = ILG;
		readDelimiter(buffer);
	} else if (status_str == "ERR") {
		status = ERR;
		readDelimiter(buffer);
	} else {
		throw InvalidMessageException();
	}
}

// ---------- ERROR MESSAGE

/**
 * @brief  Serializes a message of a err code answer made by the server.
 * @retval (stringstream) Serialized message
 */
std::stringstream ServerError::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << std::endl;
	return buffer;
}

/**
 * @brief  Reads a message of a err code answer made by the server.
 * @retval None
 */
void ServerError::readMessage(MessageAdapter &buffer) {
	// Not needed. Receiving a server error will be treated as exception.
	(void) buffer;
	return;
}

// -----------------------------------
// | Send and receive messages		 |
// -----------------------------------

/**
 * @brief Sends a message through a UDP socket.
 * @param  &message: message to send
 * @param  socketfd: udp socket file descriptor
 * @param  *addr: address to send the message to
 * @param  addrlen: size of the address
 * @param  verbose: if true, prints the message to stdout (used on server only)
 * @retval None
 */
void send_udp_message(ProtocolMessage &message, int socketfd,
                      struct sockaddr *addr, socklen_t addrlen, bool verbose) {
	const std::stringstream buffer = message.buildMessage();
	ssize_t n = sendto(socketfd, buffer.str().c_str(), buffer.str().length(), 0,
	                   addr, addrlen);
	if (n == -1) {
		throw MessageSendException();
	}
	if (verbose) {
		std::string extra = buffer.str().length() > 100 ? "..." : "";
		std::cout << "\t[INFO] Outgoing Answer (first 100 characters):\n\t-> "
				  << buffer.str().substr(0, 100) << extra << std::endl;
	}
}

/**
 * @brief  Sends a message through a TCP socket.
 * @param  &message: message to be sent
 * @param  socket_fd: TCP socket file descriptor
 * @param  verbose: if true, prints the message to stdout (used on server only)
 * @retval None
 */
void send_tcp_message(ProtocolMessage &message, int socket_fd, bool verbose) {
	std::string message_s = message.buildMessage().str();
	const char *message_str = message_s.data();
	size_t bytes_to_send = message_s.length();
	size_t bytes_sent = 0;
	while (bytes_sent < bytes_to_send) {
		ssize_t sent = write(socket_fd, message_str + bytes_sent,
		                     (size_t) (bytes_to_send - bytes_sent));
		if (sent < 0) {
			throw MessageSendException();
		}
		bytes_sent += static_cast<size_t>(sent);
	}
	if (verbose) {
		std::string extra = message_s.length() > 100 ? "..." : "";
		std::cout << "\t[INFO] Outgoing Answer (first 100 characters):\n\t-> "
				  << message_s.substr(0, 100) << extra << std::endl;
	}
}

/**
 * @brief  Waits for a UDP message to arrive and reads it.
 * @param  &message: read message
 * @param  socketfd: UDP socket file descriptor
 * @retval None
 */
void await_udp_message(ProtocolMessage &message, int socketfd) {
	fd_set file_descriptors;
	FD_ZERO(&file_descriptors);
	FD_SET(socketfd, &file_descriptors);

	struct timeval timeout;
	timeout.tv_sec = UDP_TIMEOUT;  // wait for a response before throwing
	timeout.tv_usec = 0;

	int ready_fd =
		select(socketfd + 1, &file_descriptors, NULL, NULL, &timeout);
	if (ready_fd == -1) {
		throw MessageReceiveException();
	} else if (ready_fd == 0) {
		throw MessageReceiveException();
	}

	std::stringstream data;
	char buffer[UDP_SOCKET_BUFFER_LEN];

	ssize_t n =
		recvfrom(socketfd, buffer, UDP_SOCKET_BUFFER_LEN, 0, NULL, NULL);
	if (n == -1) {
		throw MessageReceiveException();
	}

	data.write(buffer, n);
	StreamMessage strm_message(data);
	message.readMessage(strm_message);
}

/**
 * @brief  Waits for a TCP message to arrive and reads it.
 * @param  &message: read message
 * @param  socketfd: TCP socket file descriptor
 * @retval None
 */
void await_tcp_message(ProtocolMessage &message, int socketfd) {
	TcpMessage tcp_message(socketfd);
	message.readMessage(tcp_message);
}