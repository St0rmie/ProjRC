#include "protocol.hpp"

// -----------------------------------
// | Reading functions				 |
// -----------------------------------

char ProtocolMessage::readChar(std::stringstream &buffer) {
	char c;
	buffer >> c;
	if (!buffer.good()) {
		throw InvalidMessageException();
	}
	return c;
}

void ProtocolMessage::readChar(std::stringstream &buffer, char c) {
	if (readChar(buffer) != c) {
		throw InvalidMessageException();
	}
}

bool ProtocolMessage::readCharEqual(std::stringstream &buffer, char c) {
	if (readChar(buffer) != c) {
		buffer.unget();
		return false;
	} else {
		return true;
	}
}

void ProtocolMessage::readMessageId(std::stringstream &buffer,
                                    std::string protocol_code) {
	char current_char;
	int i = 0;
	while (protocol_code[i] != '\0') {
		buffer >> current_char;
		if (!buffer.good() || current_char != protocol_code[i]) {
			throw UnexpectedMessageException();
		}
		++i;
	}
}

void ProtocolMessage::readSpace(std::stringstream &buffer) {
	readChar(buffer, ' ');
}

void ProtocolMessage::readDelimiter(std::stringstream &buffer) {
	readChar(buffer, '\n');
	buffer.peek();
	if (!buffer.eof()) {
		throw InvalidMessageException();
	}
}

char ProtocolMessage::readAlphabeticalChar(std::stringstream &buffer) {
	char c = readChar(buffer);
	if (!isalpha((unsigned char) c)) {
		throw InvalidMessageException();
	}
	return (char) tolower((unsigned char) c);
}

std::string ProtocolMessage::readString(std::stringstream &buffer,
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

std::string ProtocolMessage::readAlphabeticalString(std::stringstream &buffer,
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

uint32_t ProtocolMessage::readInt(std::stringstream &buffer) {
	int64_t i;
	buffer >> i;
	if (!buffer.good() || i < 0 || i > INT32_MAX) {
		throw InvalidMessageException();
	}
	return (uint32_t) i;
}

uint32_t ProtocolMessage::readUserId(std::stringstream &buffer) {
	std::string id_str = readString(buffer, USER_ID_SIZE);
	return convert_user_id(id_str);
}

uint32_t ProtocolMessage::readAuctionId(std::stringstream &buffer) {
	std::string id_str = readString(buffer, AUCTION_ID_SIZE);
	return convert_auction_id(id_str);
}

uint32_t ProtocolMessage::readAuctionValue(std::stringstream &buffer) {
	std::string value_str = readString(buffer, AUCTION_VALUE_SIZE);
	return convert_auction_value(value_str);
}

std::string ProtocolMessage::readPassword(std::stringstream &buffer) {
	std::string password_str = readString(buffer, PASSWORD_SIZE);
	return convert_password(password_str);
}

std::string ProtocolMessage::readAuctionAndState(std::stringstream &buffer) {
	if (checkIfOver(buffer) == true) {
		return "";
	}

	readSpace(buffer);
	std::string auction_str = readString(buffer, 3);
	readSpace(buffer);
	std::string state_str = readString(buffer, 1);
	if (state_str == "1") {
		state_str = "ACTIVE";
	} else if (state_str == "0") {
		state_str = "OVER";
	}
	return "AID: " + auction_str + " --- STATUS: " + state_str;
}

bool ProtocolMessage::checkIfOver(std::stringstream &buffer) {
	char c = '\n';
	if (readChar(buffer) == c) {
		buffer.unget();
		return true;
	} else {
		buffer.unget();
		return false;
	}
}

date ProtocolMessage::readDate(std::stringstream &buffer) {
	date date;
	date.year = stoi(readString(buffer, 4));
	readChar(buffer, '-');
	date.month = stoi(readString(buffer, 2));
	readChar(buffer, '-');
	date.day = stoi(readString(buffer, 2));
	readChar(buffer, ' ');
	date.hours = stoi(readString(buffer, 2));
	readChar(buffer, ':');
	date.minutes = stoi(readString(buffer, 2));
	readChar(buffer, ':');
	date.seconds = stoi(readString(buffer, 2));

	return date;
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

void ClientLoginUser::readMessage(std::stringstream &buffer) {
	buffer >> std::noskipws;
	// Serverbound packets don't read their ID
	readSpace(buffer);
	user_id = readUserId(buffer);
	readSpace(buffer);
	password = readPassword(buffer);
	readDelimiter(buffer);
}

std::stringstream ServerLoginUser::buildMessage() {
	std::stringstream buffer;
	if (status == ServerLoginUser::status::OK) {
		buffer << "OK";
	} else if (status == ServerLoginUser::status::NOK) {
		buffer << "NOK";
	} else if (status == ServerLoginUser::status::REG) {
		buffer << "REG";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

void ServerLoginUser::readMessage(std::stringstream &buffer) {
	buffer >> std::noskipws;
	readMessageId(buffer, ServerLoginUser::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, 3);
	if (status_str == "OK") {
		status = OK;
	} else if (status_str == "NOK") {
		status = NOK;
	} else if (status_str == "REG") {
		status = REG;
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

void ClientLogout::readMessage(std::stringstream &buffer) {
	buffer >> std::noskipws;
	// Serverbound packets don't read their ID
	readSpace(buffer);
	user_id = readUserId(buffer);
	readSpace(buffer);
	password = readPassword(buffer);
	readDelimiter(buffer);
}

std::stringstream ServerLogout::buildMessage() {
	std::stringstream buffer;
	if (status == ServerLogout::status::OK) {
		buffer << "OK";
	} else if (status == ServerLogout::status::NOK) {
		buffer << "NOK";
	} else if (status == ServerLogout::status::UNR) {
		buffer << "UNR";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

void ServerLogout::readMessage(std::stringstream &buffer) {
	buffer >> std::noskipws;
	readMessageId(buffer, ServerLogout::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, 3);
	if (status_str == "OK") {
		status = OK;
	} else if (status_str == "NOK") {
		status = NOK;
	} else if (status_str == "UNR") {
		status = UNR;
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

void ClientUnregister::readMessage(std::stringstream &buffer) {
	buffer >> std::noskipws;
	// Serverbound packets don't read their ID
	readSpace(buffer);
	user_id = readUserId(buffer);
	readSpace(buffer);
	password = readPassword(buffer);
	readDelimiter(buffer);
}

std::stringstream ServerUnregister::buildMessage() {
	std::stringstream buffer;
	if (status == ServerUnregister::status::OK) {
		buffer << "OK";
	} else if (status == ServerUnregister::status::NOK) {
		buffer << "NOK";
	} else if (status == ServerUnregister::status::UNR) {
		buffer << "UNR";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

void ServerUnregister::readMessage(std::stringstream &buffer) {
	buffer >> std::noskipws;
	readMessageId(buffer, ServerUnregister::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, 3);
	if (status_str == "OK") {
		status = OK;
	} else if (status_str == "NOK") {
		status = NOK;
	} else if (status_str == "UNR") {
		status = UNR;
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

void ClientListStartedAuctions::readMessage(std::stringstream &buffer) {
	buffer >> std::noskipws;
	// Serverbound packets don't read their ID
	readSpace(buffer);
	user_id = readUserId(buffer);
	readDelimiter(buffer);
}

std::stringstream ServerListStartedAuctions::buildMessage() {
	std::stringstream buffer;
	if (status == ServerListStartedAuctions::status::OK) {
		buffer << "OK";
		for (std::string auction : auctions) {
			buffer << " " << auction;
		}
	} else if (status == ServerListStartedAuctions::status::NOK) {
		buffer << "NOK";
	} else if (status == ServerListStartedAuctions::status::NLG) {
		buffer << "NLG";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

void ServerListStartedAuctions::readMessage(std::stringstream &buffer) {
	buffer >> std::noskipws;
	readMessageId(buffer, ServerListStartedAuctions::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, 3);
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

void ClientListBiddedAuctions::readMessage(std::stringstream &buffer) {
	buffer >> std::noskipws;
	// Serverbound packets don't read their ID
	readSpace(buffer);
	user_id = readUserId(buffer);
	readDelimiter(buffer);
}

std::stringstream ServerListBiddedAuctions::buildMessage() {
	std::stringstream buffer;
	if (status == ServerListBiddedAuctions::status::OK) {
		buffer << "OK";
		for (std::string auction : auctions) {
			buffer << " " << auction;
		}
	} else if (status == ServerListBiddedAuctions::status::NOK) {
		buffer << "NOK";
	} else if (status == ServerListBiddedAuctions::status::NLG) {
		buffer << "NLG";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

void ServerListBiddedAuctions::readMessage(std::stringstream &buffer) {
	buffer >> std::noskipws;
	readMessageId(buffer, ServerListBiddedAuctions::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, 3);
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

void ClientListAllAuctions::readMessage(std::stringstream &buffer) {
	buffer >> std::noskipws;
	// Serverbound packets don't read their ID
	readDelimiter(buffer);
}

std::stringstream ServerListAllAuctions::buildMessage() {
	std::stringstream buffer;
	if (status == ServerListAllAuctions::status::OK) {
		buffer << "OK";
		for (std::string auction : auctions) {
			buffer << " " << auction;
		}
	} else if (status == ServerListAllAuctions::status::NOK) {
		buffer << "NOK";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

void ServerListAllAuctions::readMessage(std::stringstream &buffer) {
	buffer >> std::noskipws;
	readMessageId(buffer, ServerListAllAuctions::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, 3);
	if (status_str == "OK") {
		status = OK;
		std::string auc;
		while ((auc = readAuctionAndState(buffer)).length() > 0) {
			ServerListAllAuctions::auctions.push_back(auc);
		}
	} else if (status_str == "NOK") {
		status = NOK;
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

void ClientShowRecord::readMessage(std::stringstream &buffer) {
	buffer >> std::noskipws;
	readSpace(buffer);
	auction_id = readAuctionId(buffer);
	readDelimiter(buffer);
}

std::stringstream ServerShowRecord::buildMessage() {
	std::stringstream buffer;
	if (status == ServerShowRecord::status::OK) {
		buffer << "OK"
			   << "\n";
		for (bid bid : bids) {
			buffer << "B " << bid.bidder_UID;
			buffer << " " << bid.bid_value;
			buffer << " " << convert_date_to_str(bid.bid_date_time);
			buffer << " " << bid.bid_sec_time;
		}
	} else if (status == ServerShowRecord::status::NOK) {
		buffer << "NOK";
	} else {
		throw MessageBuildingException();
	}
	buffer << std::endl;
	return buffer;
}

void ServerShowRecord::readMessage(std::stringstream &buffer) {
	buffer >> std::noskipws;
	readMessageId(buffer, ServerShowRecord::protocol_code);
	readSpace(buffer);
	std::string status_str = readString(buffer, 3);
	if (status_str == "OK") {
		status = OK;
		host_UID = readUserId(buffer);
		readSpace(buffer);
		auction_name = readString(buffer, MAX_AUCTION_NAME_SIZE);
		readSpace(buffer);
		asset_fname = readString(buffer, MAX_AUCTION_NAME_SIZE);
		readSpace(buffer);
		start_value = readAuctionValue(buffer);
		readSpace(buffer);
		start_date_time = readDate(buffer);
		readSpace(buffer);
		timeactive = stoi(readString(buffer, 6));
		readDelimiter(buffer);
		while (readCharEqual(buffer, 'B')) {
			bid bid;
			readSpace(buffer);
			bid.bidder_UID = readUserId(buffer);
			readSpace(buffer);
			bid.bid_date_time = readDate(buffer);
			readSpace(buffer);
			bid.bid_sec_time = stoi(readString(buffer, 6));
			readDelimiter(buffer);
			bids.push_back(bid);
		};
		if (readCharEqual(buffer, 'E')) {
			readSpace(buffer);
			end_date_time = readDate(buffer);
			readSpace(buffer);
			end_sec_time = stoi(readString(buffer, 6));
			readDelimiter(buffer);
		}

	} else if (status_str == "NOK") {
		status = NOK;
		readDelimiter(buffer);
	} else {
		throw InvalidMessageException();
	}
	readDelimiter(buffer);
}

// -----------------------------------
// | Convert types					 |
// -----------------------------------

uint32_t convert_user_id(std::string string) {
	if (verify_user_id(string) == -1) {
		throw InvalidMessageException();
	}
	return std::stoi(string);
}

uint32_t convert_auction_id(std::string string) {
	if (verify_auction_id(string) == -1) {
		throw InvalidMessageException();
	}
	return std::stoi(string);
}

uint32_t convert_auction_value(std::string string) {
	uint32_t value = std::stoi(string);
	if (verify_value(value) == -1) {
		throw InvalidMessageException();
	}
	return value;
}

std::string convert_password(std::string string) {
	if (verify_password(string) == -1) {
		throw InvalidMessageException();
	}
	return string;
}

std::string convert_date_to_str(date date) {
	std::string date_str;
	date_str += date.year + ":" + date.month;
	date_str += ":" + date.day;
	date_str += " ";
	date_str += date.hours + ":" + date.minutes;
	date_str += ":" + date.seconds;
	return date_str;
}

// -----------------------------------
// | Send and receive messages		 |
// -----------------------------------

void send_message(ProtocolMessage &message, int socketfd, struct sockaddr *addr,
                  socklen_t addrlen) {
	const std::stringstream buffer = message.buildMessage();
	ssize_t n = sendto(socketfd, buffer.str().c_str(), buffer.str().length(), 0,
	                   addr, addrlen);
	if (n == -1) {
		throw MessageSendException();
	}
	std::cout << buffer.str() << std::endl;
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
	std::cout << data.str() << std::endl;
	message.readMessage(data);
}

void await_tcp_message(ProtocolMessage &message, int socketfd) {
	std::stringstream data;
	char buffer[SOCKET_BUFFER_LEN];
	ssize_t bytes_read;
	while ((bytes_read = recv(socketfd, &buffer, SOCKET_BUFFER_LEN, 0) > 0)) {
		if (bytes_read == -1) {
			throw MessageReceiveException();
		}

		data.write(buffer, bytes_read);
	};
	message.readMessage(data);
}