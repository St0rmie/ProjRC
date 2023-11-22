#ifndef __PROTOCOL__
#define __PROTOCOL__

#include <sstream>
#include <string>

#include "verifications.hpp"

#define USER_ID_SIZE       6
#define AUCTION_ID_SIZE    3
#define AUCTION_VALUE_SIZE 5
#define PASSWORD_SIZE      8

#define CODE_LOGIN_USER   "LIN"
#define CODE_LOGIN_SERVER "RLI"

// Thrown when the MessageID does not match what was expected
class UnexpectedMessageException : public std::runtime_error {
   public:
	UnexpectedMessageException() : std::runtime_error("Unexpected Message") {}
};

// Thrown when the MessageID is correct, but the schema is wrong
class InvalidMessageException : public std::runtime_error {
   public:
	InvalidMessageException() : std::runtime_error("Invalid Message") {}
};

// Thrown when the MessageID does not match what was expected
class MessageBuildingException : public std::runtime_error {
   public:
	MessageBuildingException()
		: std::runtime_error("Message Building error.") {}
};

class ProtocolMessage {
   protected:
	char readChar(std::stringstream &buffer);
	void readChar(std::stringstream &buffer, char chr);
	void readMessageId(std::stringstream &buffer, std::string protocol_code);
	void readSpace(std::stringstream &buffer);
	char readAlphabeticalChar(std::stringstream &buffer);
	void readDelimiter(std::stringstream &buffer);
	std::string readString(std::stringstream &buffer, uint32_t max_len);
	std::string readAlphabeticalString(std::stringstream &buffer,
	                                   uint32_t max_len);
	uint32_t readInt(std::stringstream &buffer);
	uint32_t readUserId(std::stringstream &buffer);
	uint32_t readAuctionId(std::stringstream &buffer);
	uint32_t readAuctionValue(std::stringstream &buffer);
	std::string readPassword(std::stringstream &buffer);

   public:
	virtual std::stringstream buildMessage() = 0;
	virtual void readMessage(std::stringstream &buffer) = 0;
};

// -----------------------------------
// | Client Messages for each command |
// -----------------------------------

// Login (LIN) -> UDP
class ClientLoginUser : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LOGIN_USER;
	uint32_t user_id;
	std::string password;

	std::stringstream buildMessage();
	void readMessage(std::stringstream &buffer);
};

// Close Auction (CLS) -> TCP
class ClientCloseAuction : public ProtocolMessage {
   public:
	uint32_t user_id;
	std::string password;
	uint32_t auction_id;

	void buildMessage(int fd);
	void readMessage(int fd);
};

// List started auctions by a specified user (LMA)
class ClientListStartedAuctions : public ProtocolMessage {
   public:
	uint32_t user_id;

	std::stringstream buildMessage();
	void readMessage(std::stringstream &buffer);
};

// List bidded auctions by a specified suer (LMB)
class ClientListBiddedAuction : public ProtocolMessage {
   public:
	uint32_t user_id;

	std::stringstream buildMessage();
	void readMessage(std::stringstream &buffer);
};

// List all auctions on the system (LST)
class ClientListAllAuctions : public ProtocolMessage {
   public:
	std::stringstream buildMessage();
	void readMessage(std::stringstream &buffer);
};

// Show asset of a specified auction (SAS)
class ClientShowAsset : public ProtocolMessage {
   public:
	uint32_t auction_id;

	void buildMessage(int fd);
	void readMessage(int fd);
};

// A user bids on an auction (BID)
class ClientBid : public ProtocolMessage {
   public:
	uint32_t user_id;
	std::string password;
	uint32_t auction_id;
	uint32_t value;

	void buildMessage(int fd);
	void readMessage(int fd);
};

class ClientShowRecord : public ProtocolMessage {};
class ClientLogout : public ProtocolMessage {};
class ClientUnregister : public ProtocolMessage {};

// Server Messages for each command
class ServerLoginUser : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LOGIN_SERVER;
	enum status { OK, NOK, REG };

	status status;
	std::stringstream buildMessage();
	void readMessage(std::stringstream &buffer);
};
class ServerCreateAuction : public ProtocolMessage {};
class ServerCloseAuction : public ProtocolMessage {};
class ServerListStartedAuction : public ProtocolMessage {};
class ServerListBiddedAuction : public ProtocolMessage {};
class ServerListAllAuctions : public ProtocolMessage {};
class ServerShowAsset : public ProtocolMessage {};
class ServerBid : public ProtocolMessage {};
class ServerShowRecord : public ProtocolMessage {};
class ServerLogout : public ProtocolMessage {};
class ServerUnregister : public ProtocolMessage {};

// -----------------------------------
// | Convert types					 |
// -----------------------------------

uint32_t convert_user_id(std::string string);
uint32_t convert_auction_id(std::string string);
uint32_t convert_auction_value(std::string string);
std::string convert_password(std::string string);
#endif