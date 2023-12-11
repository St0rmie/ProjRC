#ifndef __PROTOCOL__
#define __PROTOCOL__

#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "config.hpp"
#include "utils.hpp"
#include "verifications.hpp"

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
		: std::runtime_error("Could not connect to the server. Timeout.") {}
};

class MessageAdapter {
   public:
	virtual char get() = 0;
	virtual bool good() = 0;
	virtual void unget() = 0;
	virtual std::string getn(int n) = 0;
};

class StreamMessage : public MessageAdapter {
   private:
	std::stringstream &_stream;

   public:
	StreamMessage(std::stringstream &stream) : _stream(stream){};
	char get() {
		return (char) _stream.get();
	};
	bool good() {
		return _stream.good();
	};
	void unget() {
		_stream.unget();
	};
	std::string getn(int n) {
		std::string str;
		str.resize(n);
		_stream.read(&str[0], n);
		return str;
	}
};

class TcpMessage : public MessageAdapter {
   private:
	int _fd;
	std::vector<char> _buffer;
	char _last;

   public:
	TcpMessage(int fd) : _fd(fd){};
	void fillBuffer() {
		char buf[SOCKET_BUFFER_LEN];
		ssize_t n = read(_fd, &buf, SOCKET_BUFFER_LEN);

		if (n <= 0) {
			throw InvalidMessageException();
		}

		for (int i = n - 1; i >= 0; i--) {
			_buffer.push_back(buf[i]);
		}
	}

	char get() {
		if (_buffer.size() == 0) {
			fillBuffer();
		}
		_last = _buffer.back();
		_buffer.pop_back();

		return _last;
	};

	bool good() {
		return true;
	};

	void unget() {
		_buffer.push_back(_last);
	};

	std::string getn(int n) {
		std::string str;
		for (int i = 0; i < n; i++) {
			str.push_back(get());
		}
		return str;
	}
};

class ProtocolMessage {
   protected:
	char readChar(MessageAdapter &buffer);
	void readChar(MessageAdapter &buffer, char chr);
	bool readCharEqual(MessageAdapter &buffer, char chr);
	void readMessageId(MessageAdapter &buffer, std::string protocol_code);
	void readSpace(MessageAdapter &buffer);
	char readAlphabeticalChar(MessageAdapter &buffer);
	void readDelimiter(MessageAdapter &buffer);
	std::string readString(MessageAdapter &buffer, uint32_t max_len);
	std::string readAlphabeticalString(MessageAdapter &buffer,
	                                   uint32_t max_len);
	uint32_t readInt(MessageAdapter &buffer);
	uint32_t readUserId(MessageAdapter &buffer);
	uint32_t readAuctionId(MessageAdapter &buffer);
	uint32_t readAuctionValue(MessageAdapter &buffer);
	std::string readPassword(MessageAdapter &buffer);
	std::string readAuctionAndState(MessageAdapter &buffer);
	bool checkIfOver(MessageAdapter &buffer);
	datetime readDate(MessageAdapter &buffer);
	void parseDate(datetime date, std::string date_str);
	std::string readFile(MessageAdapter &buffer, uint32_t max_len);

   public:
	virtual std::stringstream buildMessage() = 0;
	virtual void readMessage(MessageAdapter &buffer) = 0;
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
	void readMessage(MessageAdapter &buffer);
};

// Logout (LOU) -> UDP
class ClientLogout : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LOGOUT_USER;
	uint32_t user_id;
	std::string password;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

class ClientUnregister : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_UNREGISTER_USER;
	uint32_t user_id;
	std::string password;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

// List started auctions by a specified user (LMA)
class ClientListStartedAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_AUC_USER;
	uint32_t user_id;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

// List bidded auctions by a specified suer (LMB)
class ClientListBiddedAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_MYB_USER;
	uint32_t user_id;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

// List all auctions on the system (LST)
class ClientListAllAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_ALLAUC_USER;
	uint32_t user_id;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

// Show record of an auction on the system
class ClientShowRecord : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_SHOWREC_USER;
	uint32_t auction_id;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

class ClientOpenAuction : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_OPEN_AUC_CLIENT;
	uint32_t user_id;
	std::string password;
	uint32_t start_value;
	uint32_t timeactive;
	std::string name;
	std::string assetf_name;
	size_t fsize;
	std::string fdata;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

// Close Auction (CLS) -> TCP
class ClientCloseAuction : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_CLOSE_AUC_CLIENT;
	uint32_t user_id;
	std::string password;
	uint32_t auction_id;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

// Show asset of a specified auction (SAS)
class ClientShowAsset : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_SHOW_ASSET_CLIENT;
	uint32_t auction_id;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

// A user bids on an auction (BID)
class ClientBid : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_BID_CLIENT;
	uint32_t user_id;
	std::string password;
	uint32_t auction_id;
	uint32_t value;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

// Server Messages for each command
class ServerLoginUser : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LOGIN_SERVER;
	enum status { OK, NOK, REG, ERR };

	status status;
	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

class ServerLogout : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LOGOUT_SERVER;
	enum status { OK, NOK, UNR, ERR };

	status status;
	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

class ServerUnregister : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_UNREGISTER_SERVER;
	enum status { OK, NOK, UNR, ERR };

	status status;
	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

class ServerListStartedAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_AUC_SERVER;
	enum status { OK, NOK, NLG, ERR };
	std::vector<std::string> auctions;

	status status;
	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

class ServerListBiddedAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_MYB_SERVER;
	enum status { OK, NOK, NLG, ERR };
	std::vector<std::string> auctions;

	status status;
	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

class ServerListAllAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_ALLAUC_SERVER;
	enum status { OK, NOK, ERR };
	std::vector<std::string> auctions;

	status status;
	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

class ServerShowRecord : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_SHOWREC_SERVER;
	enum status { OK, NOK, ERR };
	uint32_t host_UID;
	std::string auction_name;
	std::string asset_fname;
	uint32_t start_value;
	datetime start_date_time;
	uint32_t timeactive;
	std::vector<bid> bids;
	datetime end_date_time;
	uint32_t end_sec_time = 0;
	status status;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

class ServerOpenAuction : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_OPEN_AUC_SERVER;
	enum status { OK, NOK, NLG, ERR };
	uint32_t auction_id;
	status status;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

class ServerCloseAuction : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_CLOSE_AUC_SERVER;
	enum status { OK, NLG, EAU, EOW, END, ERR };
	status status;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

class ServerShowAsset : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_SHOW_ASSET_SERVER;
	enum status { OK, NOK, ERR };
	std::string fname;
	size_t fsize;
	std::string fdata;
	status status;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

class ServerBid : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_BID_SERVER;
	enum status { NOK, NLG, ACC, REF, ILG, ERR };
	status status;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

class ServerError : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_BID_SERVER;
	enum status { ERR };
	status status = ERR;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

// -----------------------------------
// | Send and receive messages		 |
// -----------------------------------

void send_udp_message(ProtocolMessage &message, int socket,
                      struct sockaddr *address, socklen_t addrlen,
                      bool verbose);
void send_tcp_message(ProtocolMessage &message, int socketfd,
                      struct sockaddr *addr, socklen_t addrlen, bool verbose);

void await_udp_message(ProtocolMessage &Message, int socketfd);
void await_tcp_message(ProtocolMessage &Message, int socketfd);
#endif