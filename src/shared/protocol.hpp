#ifndef __PROTOCOL__
#define __PROTOCOL__

#include <sys/socket.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "verifications.hpp"

#define DEFAULT_HOSTNAME "localhost"
#define DEFAULT_PORT     "58086"

#define USER_ID_SIZE       6
#define AUCTION_ID_SIZE    3
#define AUCTION_VALUE_SIZE 5
#define PASSWORD_SIZE      8

#define CODE_LOGIN_USER         "LIN"
#define CODE_LOGIN_SERVER       "RLI"
#define CODE_LOGOUT_USER        "LOU"
#define CODE_LOGOUT_SERVER      "RLO"
#define CODE_UNREGISTER_USER    "UNR"
#define CODE_UNREGISTER_SERVER  "RUR"
#define CODE_LIST_AUC_USER      "LMA"
#define CODE_LIST_AUC_SERVER    "RMA"
#define CODE_LIST_MYB_USER      "LMB"
#define CODE_LIST_MYB_SERVER    "RMB"
#define CODE_LIST_ALLAUC_USER   "LST"
#define CODE_LIST_ALLAUC_SERVER "RLS"
#define CODE_SHOWREC_USER       "SRC"
#define CODE_SHOWREC_SERVER     "RRC"

#define UDP_TIMEOUT   5
#define UDP_MAX_TRIES 5

#define TCP_READ_TIMEOUT_SECONDS   20
#define TCP_READ_TIMEOUT_USECONDS  0
#define TCP_WRITE_TIMEOUT_SECONDS  60
#define TCP_WRITE_TIMEOUT_USECONDS 0

#define SOCKET_BUFFER_LEN 512

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

// Thrown when the Message couldn't be sent
class MessageSendException : public std::runtime_error {
   public:
	MessageSendException() : std::runtime_error("Message couldn't be sent.") {}
};

// Thrown when the Message couldn't be sent
class MessageReceiveException : public std::runtime_error {
   public:
	MessageReceiveException()
		: std::runtime_error("Message couldn't be received.") {}
};

// Thrown when reading/writing packet exceeds the timeout time.
class ConnectionTimeoutException : public std::runtime_error {
   public:
	ConnectionTimeoutException()
		: std::runtime_error("Could not connect to the game server. Timeout.") {
	}
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
	std::string readAuctionAndState(std::stringstream &buffer);
	bool checkIfOver(std::stringstream &buffer);

   public:
	virtual std::stringstream buildMessage() = 0;
	virtual void readMessage(std::stringstream &buffer) = 0;
};

class UdpMessage : public ProtocolMessage {
   public:
	virtual std::stringstream buildMessage() = 0;
	virtual void readMessage(std::stringstream &buffer) = 0;
};

class TcpMessage : public ProtocolMessage {
   public:
	virtual std::stringstream buildMessageAndWrite(int fd) = 0;
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

// Logout (LOU) -> UDP
class ClientLogout : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LOGOUT_USER;
	uint32_t user_id;
	std::string password;

	std::stringstream buildMessage();
	void readMessage(std::stringstream &buffer);
};

class ClientUnregister : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_UNREGISTER_USER;
	uint32_t user_id;
	std::string password;

	std::stringstream buildMessage();
	void readMessage(std::stringstream &buffer);
};

// List started auctions by a specified user (LMA)
class ClientListStartedAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_AUC_USER;
	uint32_t user_id;

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

// List bidded auctions by a specified suer (LMB)
class ClientListBiddedAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_MYB_USER;
	uint32_t user_id;

	std::stringstream buildMessage();
	void readMessage(std::stringstream &buffer);
};

// List all auctions on the system (LST)
class ClientListAllAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_ALLAUC_USER;
	uint32_t user_id;

	std::stringstream buildMessage();
	void readMessage(std::stringstream &buffer);
};

// Show record of an auction on the system
class ClientShowRecord : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_SHOWREC_USER;
	uint32_t auction_id;

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

// Server Messages for each command
class ServerLoginUser : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LOGIN_SERVER;
	enum status { OK, NOK, REG };

	status status;
	std::stringstream buildMessage();
	void readMessage(std::stringstream &buffer);
};

class ServerLogout : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LOGOUT_SERVER;
	enum status { OK, NOK, UNR };

	status status;
	std::stringstream buildMessage();
	void readMessage(std::stringstream &buffer);
};

class ServerUnregister : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_UNREGISTER_SERVER;
	enum status { OK, NOK, UNR };

	status status;
	std::stringstream buildMessage();
	void readMessage(std::stringstream &buffer);
};

class ServerListStartedAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_AUC_SERVER;
	enum status { OK, NOK, NLG };
	std::vector<std::string> auctions;

	status status;
	std::stringstream buildMessage();
	void readMessage(std::stringstream &buffer);
};

class ServerListBiddedAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_MYB_SERVER;
	enum status { OK, NOK, NLG };
	std::vector<std::string> auctions;

	status status;
	std::stringstream buildMessage();
	void readMessage(std::stringstream &buffer);
};

class ServerListAllAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_MYB_SERVER;
	enum status { OK, NOK };
	std::vector<std::string> auctions;

	status status;
	std::stringstream buildMessage();
	void readMessage(std::stringstream &buffer);
};

class ServerShowRecord : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_SHOWREC_SERVER;
	enum status { OK, NOK };
	std::string auction;

	status status;
	std::stringstream buildMessage();
	void readMessage(std::stringstream &buffer);
};

class ServerCreateAuction : public ProtocolMessage {};
class ServerCloseAuction : public ProtocolMessage {};
class ServerShowAsset : public ProtocolMessage {};
class ServerBid : public ProtocolMessage {};

// -----------------------------------
// | Convert types					 |
// -----------------------------------

uint32_t convert_user_id(std::string string);
uint32_t convert_auction_id(std::string string);
uint32_t convert_auction_value(std::string string);
std::string convert_password(std::string string);

// -----------------------------------
// | Send and receive messages		 |
// -----------------------------------

void send_message(ProtocolMessage &message, int socket,
                  struct sockaddr *address, socklen_t addrlen);

void await_udp_message(ProtocolMessage &Message, int socketfd);
void await_tcp_message(ProtocolMessage &Message, int socketfd);
#endif