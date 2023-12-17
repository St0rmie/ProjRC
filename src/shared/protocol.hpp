#ifndef __PROTOCOL__
#define __PROTOCOL__

/**
 * @file protocol.hpp
 * @brief This file contains the definition of the Protocol used in the
 * communication between the Auction Server and the user Client.
 */

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

// -----------------------------------
// | Protocol Codes                  |
// -----------------------------------

#define CODE_LOGIN_USER   "LIN"
#define CODE_LOGIN_SERVER "RLI"

#define CODE_LOGOUT_USER   "LOU"
#define CODE_LOGOUT_SERVER "RLO"

#define CODE_UNREGISTER_USER   "UNR"
#define CODE_UNREGISTER_SERVER "RUR"

#define CODE_LIST_AUC_USER   "LMA"
#define CODE_LIST_AUC_SERVER "RMA"

#define CODE_LIST_MYB_USER   "LMB"
#define CODE_LIST_MYB_SERVER "RMB"

#define CODE_LIST_ALLAUC_USER   "LST"
#define CODE_LIST_ALLAUC_SERVER "RLS"

#define CODE_SHOWREC_USER   "SRC"
#define CODE_SHOWREC_SERVER "RRC"

#define CODE_OPEN_AUC_CLIENT "OPA"
#define CODE_OPEN_AUC_SERVER "ROA"

#define CODE_CLOSE_AUC_CLIENT "CLS"
#define CODE_CLOSE_AUC_SERVER "RCL"

#define CODE_SHOW_ASSET_CLIENT "SAS"
#define CODE_SHOW_ASSET_SERVER "RSA"

#define CODE_BID_CLIENT "BID"
#define CODE_BID_SERVER "RBD"

#define CODE_ERROR "ERR"

// -----------------------------------
// | Exceptions                      |
// -----------------------------------

/**
 * @brief  Thrown when the MessageID does not match what was expected
 */
class UnexpectedMessageException : public std::runtime_error {
   public:
	UnexpectedMessageException()
		: std::runtime_error("[ERROR] Unexpected Message.") {}
};

/**
 * @brief  Thrown when the MessageID matches the error code
 */
class ERRCodeMessageException : public std::runtime_error {
   public:
	ERRCodeMessageException() : std::runtime_error("Error code.") {}
};

/**
 * @brief  Thrown when the MessageID is correct, but the format is wrong
 */
class InvalidMessageException : public std::runtime_error {
   public:
	InvalidMessageException()
		: std::runtime_error("[ERROR] Invalid Message.") {}
};

/**
 * @brief  Thrown when the MessageID does not match what was expected
 */
class MessageBuildingException : public std::runtime_error {
   public:
	MessageBuildingException()
		: std::runtime_error("[ERROR] Message Building error.") {}
};

/**
 * @brief  Thrown when the message couldn't be sent
 */
class MessageSendException : public std::runtime_error {
   public:
	MessageSendException() : std::runtime_error("Message couldn't be sent.") {}
};

/**
 * @brief  Thrown when the message couldn't be received
 */
class MessageReceiveException : public std::runtime_error {
   public:
	MessageReceiveException()
		: std::runtime_error("Message couldn't be received.") {}
};

/**
 * @brief  Thrown when reading/writing message exceeds the timeout time.
 */
class ConnectionTimeoutException : public std::runtime_error {
   public:
	ConnectionTimeoutException()
		: std::runtime_error("Could not connect to the server. Timeout.") {}
};

// -----------------------------------
// | Classes   	                     |
// -----------------------------------

/**
 * @brief  This class is an adapter that generalizes the reading of messages. It
 * gives an interface that is common between reading from a TCP socket or a
 * stringstream (UDP). This way, the same reading process to decode the message
 * can be used independently of the type of message.
 */
class MessageAdapter {
   public:
	virtual char get() = 0;
	virtual bool good() = 0;
	virtual void unget() = 0;
	virtual std::string getn(int n) = 0;
};

/**
 * @brief  This class is the specification of the Adapter for the case of
 * reading a UDP message from a stringstream. The methods used almost correspond
 * to those of the stringstream.
 */
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
		size_t size = static_cast<size_t>(n);
		std::string str;
		str.resize(size);
		_stream.read(&str[0], n);
		return str;
	}
};

/**
 * @brief  This class is the specification of the Adapter for the case of
 * reading a TCP message from the socket. The methods used read directly from
 * the socket or from a buffer that is used to simulate the behaviour of a
 * stringstream.
 */
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

		// The vector is filled in reverse order so that the pop_back() method
		// retrieves the next character from the beginning of the message.
		for (ssize_t i = n - 1; i >= 0; i--) {
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

/**
 * @brief  This class is the generalization of a Protocol Message. It specifies
 * all the methods required to read a message properly given a message adapter.
 * It also specifies the methods to build a message and to read it.
 */
class ProtocolMessage {
   protected:
	char readChar(MessageAdapter &buffer);
	void readChar(MessageAdapter &buffer, char chr);
	bool readCharEqual(MessageAdapter &buffer, char chr);
	void readMessageId(MessageAdapter &buffer, std::string protocol_code);
	void readSpace(MessageAdapter &buffer);
	void readDelimiter(MessageAdapter &buffer);
	std::string readString(MessageAdapter &buffer, uint32_t max_len);
	uint32_t readUserId(MessageAdapter &buffer);
	uint32_t readAuctionId(MessageAdapter &buffer);
	uint32_t readAuctionValue(MessageAdapter &buffer);
	std::string readPassword(MessageAdapter &buffer);
	std::string readAuctionAndState(MessageAdapter &buffer);
	bool checkIfOver(MessageAdapter &buffer);
	Datetime readDate(MessageAdapter &buffer);
	void parseDate(Datetime date, std::string date_str);
	std::string readFile(MessageAdapter &buffer, uint32_t max_len);

   public:
	// Creates a string stream with the formatted message
	virtual std::stringstream buildMessage() = 0;

	// Fills the instance of the class with data from read from the adapter
	virtual void readMessage(MessageAdapter &buffer) = 0;
};

// -----------------------------------
// | Client Messages for each command |
// -----------------------------------

/**
 * @brief  Login (LIN) -> UDP.
 * Message sent by the client to the server representing a login command.
 * */
class ClientLoginUser : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LOGIN_USER;
	uint32_t user_id;
	std::string password;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  Logout (LOU) -> UDP
 * Message sent by the client to the server representing a logout command.
 */
class ClientLogout : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LOGOUT_USER;
	uint32_t user_id;
	std::string password;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  Unregister (UNR) -> UDP
 * Message sent by the client to the server representing a unregister command.
 */
class ClientUnregister : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_UNREGISTER_USER;
	uint32_t user_id;
	std::string password;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  List Started Auctions (LMA) -> UDP
 * Message sent by the client to the server representing a List My Auctions
 * command.
 */
class ClientListStartedAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_AUC_USER;
	uint32_t user_id;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  List My Bids (LMB) -> UDP
 * Message sent by the client to the server representing a List My Bids command.
 */
class ClientListBiddedAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_MYB_USER;
	uint32_t user_id;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  List (LST) -> UDP
 * Message sent by the client to the server representing a List All Auctions
 * command.
 */
class ClientListAllAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_ALLAUC_USER;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  Show Record (SRC) -> UDP
 * Message sent by the client to the server representing a Show Record command.
 */
class ClientShowRecord : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_SHOWREC_USER;
	uint32_t auction_id;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  Open Auction (OPA) -> TCP
 * Message sent by the client to the server representing a open auction command.
 */
class ClientOpenAuction : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_OPEN_AUC_CLIENT;
	uint32_t user_id;
	std::string password;
	uint32_t start_value;
	uint32_t timeactive;
	std::string name;
	std::string assetf_name;
	size_t Fsize;
	std::string fdata;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  Close Auction (CLS) -> TCP
 * Message sent by the client to the server representing a Close Auction
 * command.
 */
class ClientCloseAuction : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_CLOSE_AUC_CLIENT;
	uint32_t user_id;
	std::string password;
	uint32_t auction_id;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  Show Asset (SAS) -> TCP
 * Message sent by the client to the server representing a Show Asset command.
 */
class ClientShowAsset : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_SHOW_ASSET_CLIENT;
	uint32_t auction_id;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  Bid (BID) -> UDP
 * Message sent by the client to the server representing a Bid command.
 */
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

// ------------------------------------
// | Server Messages for each command.|
// ------------------------------------

/**
 * @brief  Login (RLI) -> UDP
 * Message sent by the server to the client representing the answer to a Login
 * command.
 */
class ServerLoginUser : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LOGIN_SERVER;
	enum status { OK, NOK, REG, ERR };

	status status;
	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  Logout (RLO) -> UDP
 * Message sent by the server to the client representing the answer to a Logout
 * command.
 */
class ServerLogout : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LOGOUT_SERVER;
	enum status { OK, NOK, UNR, ERR };

	status status;
	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  Unregister (RUR) -> UDP
 * Message sent by the server to the client representing the answer to a Bid
 * command.
 */
class ServerUnregister : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_UNREGISTER_SERVER;
	enum status { OK, NOK, UNR, ERR };

	status status;
	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  List My Auctions (RMA) -> UDP
 * Message sent by the server to the client representing the answer to a List My
 * Auctions command.
 */
class ServerListStartedAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_AUC_SERVER;
	enum status { OK, NOK, NLG, ERR };
	std::vector<std::string> auctions;

	status status;
	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  List My Bids (RMB) -> UDP
 * Message sent by the server to the client representing the answer to a List My
 * Bids command.
 */
class ServerListBiddedAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_MYB_SERVER;
	enum status { OK, NOK, NLG, ERR };
	std::vector<std::string> auctions;

	status status;
	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  List All Auctions (RLS) -> UDP
 * Message sent by the server to the client representing the answer to a List
 * All Auctions command.
 */
class ServerListAllAuctions : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_LIST_ALLAUC_SERVER;
	enum status { OK, NOK, ERR };
	std::vector<std::string> auctions;

	status status;
	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  Show Record (RRC) -> UDP
 * Message sent by the server to the client representing the answer to a Show
 * Record command.
 */
class ServerShowRecord : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_SHOWREC_SERVER;
	enum status { OK, NOK, ERR };
	uint32_t host_UID;
	std::string auction_name;
	std::string asset_fname;
	uint32_t start_value;
	Datetime start_date_time;
	uint32_t timeactive;
	std::vector<Bid> bids;
	Datetime end_date_time;
	uint32_t end_sec_time = 0;
	status status;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief Open Auction (ROA) -> TCP
 * Message sent by the server to the client representing the answer to a Open
 * Auction command.
 */
class ServerOpenAuction : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_OPEN_AUC_SERVER;
	enum status { OK, NOK, NLG, ERR };
	uint32_t auction_id;
	status status;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief Close Auction (RCL) -> TCP
 * Message sent by the server to the client representing the answer to a Close
 * Auction command.
 */
class ServerCloseAuction : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_CLOSE_AUC_SERVER;
	enum status { OK, NLG, EAU, EOW, END, ERR, NOK };
	status status;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  Show Asset (RSA) -> TCP
 * Message sent by the server to the client representing the answer to a Show
 * Asset command.
 */
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

/**
 * @brief Bid (RBD) -> TCP
 * Message sent by the server to the client representing the answer to a Bid
 * command.
 */
class ServerBid : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_BID_SERVER;
	enum status { NOK, NLG, ACC, REF, ILG, ERR };
	status status;

	std::stringstream buildMessage();
	void readMessage(MessageAdapter &buffer);
};

/**
 * @brief  Error (ERR) -> UDP & TCP
 * Message sent by the server to the client representing the answer to a
 * command that resulted in an error.
 */
class ServerError : public ProtocolMessage {
   public:
	std::string protocol_code = CODE_ERROR;
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
void await_udp_message(ProtocolMessage &Message, int socketfd);
void send_tcp_message(ProtocolMessage &message, int socketfd, bool verbose);
void await_tcp_message(ProtocolMessage &Message, int socketfd);
#endif