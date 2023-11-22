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
	uint32_t i = 0;
	for (uint32_t i; i < max_len; i++) {
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

// -----------------------------------
// | Convert types					 |
// -----------------------------------

uint32_t convert_user_id(std::string string) {
	if (verify_user_id(string) == 0) {
		throw InvalidMessageException();
	}
	return std::stoi(string);
}

uint32_t convert_auction_id(std::string string) {
	if (verify_auction_id(string) == 0) {
		throw InvalidMessageException();
	}
	return std::stoi(string);
}

uint32_t convert_auction_value(std::string string) {
	uint32_t value = std::stoi(string);
	if (verify_value(value) == 0) {
		throw InvalidMessageException();
	}
	return value;
}

std::string convert_password(std::string string) {
	if (verify_password(string) == 0) {
		throw InvalidMessageException();
	}
	return string;
}