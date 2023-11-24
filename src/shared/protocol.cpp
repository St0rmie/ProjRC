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
		// Read Auction
	} else if (status_str == "NOK") {
		status = NOK;
	} else if (status_str == "NLG") {
		status = NLG;
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
	char buffer[SOCKET_BUFFER_LEN];

	ssize_t n = recvfrom(socketfd, buffer, SOCKET_BUFFER_LEN, 0, NULL, NULL);
	if (n == -1) {
		throw MessageReceiveException();
	}

	data.write(buffer, n);
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