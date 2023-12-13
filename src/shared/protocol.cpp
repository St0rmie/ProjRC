#include "protocol.hpp"

// -----------------------------------
// | Reading functions				 |
// -----------------------------------

char ProtocolMessage::readChar(MessageAdapter &buffer) {
	char c;
	c = buffer.get();
	if (!buffer.good()) {
		throw InvalidMessageException();
	}
	return c;
}

void ProtocolMessage::readChar(MessageAdapter &buffer, char c) {
	char read = readChar(buffer);
	if (read != c) {
		throw InvalidMessageException();
	}
}

bool ProtocolMessage::readCharEqual(MessageAdapter &buffer, char c) {
	char read = readChar(buffer);
	if (read != c) {
		buffer.unget();
		return false;
	} else {
		return true;
	}
}

void ProtocolMessage::readMessageId(MessageAdapter &buffer,
                                    std::string protocol_code) {
	char current_char;
	int i = 0;
	while (protocol_code[i] != '\0') {
		current_char = readChar(buffer);
		if (!buffer.good() || current_char != protocol_code[i]) {
			throw UnexpectedMessageException();
		}
		++i;
	}
}

void ProtocolMessage::readSpace(MessageAdapter &buffer) {
	readChar(buffer, ' ');
}

void ProtocolMessage::readDelimiter(MessageAdapter &buffer) {
	readChar(buffer, '\n');
}

char ProtocolMessage::readAlphabeticalChar(MessageAdapter &buffer) {
	char c = readChar(buffer);
	if (!isalpha((unsigned char) c)) {
		throw InvalidMessageException();
	}
	return (char) tolower((unsigned char) c);
}

std::string ProtocolMessage::readString(MessageAdapter &buffer,
                                        uint32_t max_len) {
	std::string str;
	for (uint32_t i = 0; i < max_len; i++) {
		char c = (char) buffer.get();
		if (!buffer.good()) {
			throw InvalidMessageException();
		}
		if (c == ' ' || c == '\n') {
			buffer.unget();
			break;
		}
		str += c;
	}

	return str;
}

std::string ProtocolMessage::readAlphabeticalString(MessageAdapter &buffer,
                                                    uint32_t max_len) {
	std::string str = readString(buffer, max_len);
	for (uint32_t i = 0; i < str.length(); ++i) {
		if (!isalpha((unsigned char) str[i])) {
			throw InvalidMessageException();
		}

		str[i] = (char) tolower((unsigned char) str[i]);
	}
	return str;
}

uint32_t ProtocolMessage::readInt(MessageAdapter &buffer) {
	int64_t i = buffer.get();
	if (!buffer.good() || i < 0 || i > INT32_MAX) {
		throw InvalidMessageException();
	}
	return (uint32_t) i;
}

uint32_t ProtocolMessage::readUserId(MessageAdapter &buffer) {
	std::string id_str = readString(buffer, USER_ID_SIZE);
	return convert_user_id(id_str);
}

uint32_t ProtocolMessage::readAuctionId(MessageAdapter &buffer) {
	std::string id_str = readString(buffer, AUCTION_ID_SIZE);
	return convert_auction_id(id_str);
}

uint32_t ProtocolMessage::readAuctionValue(MessageAdapter &buffer) {
	std::string value_str = readString(buffer, MAX_AUCTION_VALUE_SIZE);
	return convert_auction_value(value_str);
}

std::string ProtocolMessage::readPassword(MessageAdapter &buffer) {
	std::string password_str = readString(buffer, PASSWORD_SIZE);
	return convert_password(password_str);
}

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

datetime ProtocolMessage::readDate(MessageAdapter &buffer) {
	datetime date;
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

std::stringstream ClientLoginUser::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " " << user_id << " " << password << std::endl;
	return buffer;
}

void ClientLoginUser::readMessage(MessageAdapter &buffer) {
	// Serverbound packets don't read their ID
	readSpace(buffer);
	user_id = readUserId(buffer);
	readSpace(buffer);
	password = readPassword(buffer);
	readDelimiter(buffer);
}

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

std::stringstream ClientLogout::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " " << user_id << " " << password << std::endl;
	return buffer;
}

void ClientLogout::readMessage(MessageAdapter &buffer) {
	// Serverbound packets don't read their ID
	readSpace(buffer);
	user_id = readUserId(buffer);
	readSpace(buffer);
	password = readPassword(buffer);
	readDelimiter(buffer);
}

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

std::stringstream ClientUnregister::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " " << user_id << " " << password << std::endl;
	return buffer;
}

void ClientUnregister::readMessage(MessageAdapter &buffer) {
	// Serverbound packets don't read their ID
	readSpace(buffer);
	user_id = readUserId(buffer);
	readSpace(buffer);
	password = readPassword(buffer);
	readDelimiter(buffer);
}

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

std::stringstream ClientListStartedAuctions::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " " << user_id << std::endl;
	return buffer;
}

void ClientListStartedAuctions::readMessage(MessageAdapter &buffer) {
	// Serverbound packets don't read their ID
	readSpace(buffer);
	user_id = readUserId(buffer);
	readDelimiter(buffer);
}

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

std::stringstream ClientListBiddedAuctions::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " " << user_id << std::endl;
	return buffer;
}

void ClientListBiddedAuctions::readMessage(MessageAdapter &buffer) {
	// Serverbound packets don't read their ID
	readSpace(buffer);
	user_id = readUserId(buffer);
	readDelimiter(buffer);
}

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

std::stringstream ClientListAllAuctions::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << std::endl;
	return buffer;
}

void ClientListAllAuctions::readMessage(MessageAdapter &buffer) {
	// Serverbound packets don't read their ID
	readDelimiter(buffer);
}

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

std::stringstream ClientShowRecord::buildMessage() {
	std::stringstream buffer;
	char aid[4];
	sprintf(aid, "%03d", auction_id);
	buffer << protocol_code << " " << aid << std::endl;
	return buffer;
}

void ClientShowRecord::readMessage(MessageAdapter &buffer) {
	readSpace(buffer);
	auction_id = readAuctionId(buffer);
	readDelimiter(buffer);
}

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
		for (bid bid : bids) {
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

void ServerShowRecord::readMessage(MessageAdapter &buffer) {
	bool skip;
	readMessageId(buffer, ServerShowRecord::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, MAX_STATUS_SIZE);
	readSpace(buffer);
	if (status_str == "OK") {
		status = OK;
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
		timeactive = stoi(readString(buffer, MAX_TIMEACTIVE_SIZE));
		if (readCharEqual(buffer, ' ')) {
			while (readCharEqual(buffer, 'B')) {
				bid bid;
				readSpace(buffer);
				bid.bidder_UID = readUserId(buffer);
				readSpace(buffer);
				bid.bid_value =
					stoi(readString(buffer, MAX_AUCTION_VALUE_SIZE));
				readSpace(buffer);
				bid.bid_date_time = readDate(buffer);
				readSpace(buffer);
				bid.bid_sec_time =
					stoi(readString(buffer, MAX_LENGTH_TIMEACTIVE));
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
				end_sec_time = stoi(readString(buffer, MAX_LENGTH_TIMEACTIVE));
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

std::stringstream ClientOpenAuction::buildMessage() {
	std::stringstream buffer;
	buffer << protocol_code << " " << user_id << " " << password << " " << name
		   << " " << start_value << " " << timeactive << " " << assetf_name
		   << " " << fsize << " " << fdata << std::endl;
	return buffer;
}

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
	timeactive = stoi(readString(buffer, MAX_LENGTH_TIMEACTIVE));
	readSpace(buffer);
	assetf_name = readString(buffer, MAX_FILENAME_SIZE);
	readSpace(buffer);
	size_t fsize = (size_t) stoi(readString(buffer, MAX_FILE_SIZE_LENGTH));
	readSpace(buffer);
	fdata = readFile(buffer, fsize);
	readDelimiter(buffer);
}

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

std::stringstream ClientCloseAuction::buildMessage() {
	std::stringstream buffer;
	char aid[4];
	sprintf(aid, "%03d", auction_id);
	buffer << protocol_code << " " << user_id << " " << password << " " << aid
		   << std::endl;
	return buffer;
}

void ClientCloseAuction::readMessage(MessageAdapter &buffer) {
	readSpace(buffer);
	user_id = readUserId(buffer);
	readSpace(buffer);
	password = readPassword(buffer);
	readSpace(buffer);
	auction_id = readAuctionId(buffer);
	readDelimiter(buffer);
}

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
	} else if (status_str == "ERR") {
		status = ERR;
	} else {
		throw InvalidMessageException();
	}
}

// ---------- SHOW ASSET

std::stringstream ClientShowAsset::buildMessage() {
	std::stringstream buffer;
	char aid[4];
	sprintf(aid, "%03d", auction_id);
	buffer << protocol_code << " " << aid << std::endl;
	return buffer;
}

void ClientShowAsset::readMessage(MessageAdapter &buffer) {
	readSpace(buffer);
	auction_id = readAuctionId(buffer);
	readDelimiter(buffer);
}

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

void ServerShowAsset::readMessage(MessageAdapter &buffer) {
	readMessageId(buffer, ServerShowAsset::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, MAX_STATUS_SIZE);
	if (status_str == "OK") {
		status = OK;
		readSpace(buffer);
		fname = readString(buffer, MAX_FILENAME_SIZE);
		readSpace(buffer);
		fsize = stol(readString(buffer, MAX_FILE_SIZE_LENGTH));
		readSpace(buffer);
		fdata = readFile(buffer, fsize);
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

std::stringstream ClientBid::buildMessage() {
	std::stringstream buffer;
	char aid[4];
	sprintf(aid, "%03d", auction_id);
	buffer << protocol_code << " " << user_id << " " << password << " " << aid
		   << " " << value << std::endl;
	return buffer;
}

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

// -----------------------------------
// | Send and receive messages		 |
// -----------------------------------

void send_udp_message(ProtocolMessage &message, int socketfd,
                      struct sockaddr *addr, socklen_t addrlen, bool verbose) {
	const std::stringstream buffer = message.buildMessage();
	ssize_t n = sendto(socketfd, buffer.str().c_str(), buffer.str().length(), 0,
	                   addr, addrlen);
	if (n == -1) {
		throw MessageSendException();
	}
	if (verbose) {
		std::cout << "\t --> MESSAGE: " << buffer.str() << std::endl;
	}
}

void send_tcp_message(ProtocolMessage &message, int socket_fd,
                      struct sockaddr *addr, socklen_t addrlen, bool verbose) {
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
		bytes_sent += sent;
	}
	if (verbose) {
		std::cout << "\t --> MESSAGE: " << message_s << std::endl;
	}
}

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

void await_tcp_message(ProtocolMessage &message, int socketfd) {
	TcpMessage tcp_message(socketfd);
	message.readMessage(tcp_message);
}