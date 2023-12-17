#ifndef __SERVER_HANDLERS__
#define __ SERVER_HANDLERS__

/**
 * @file client.hpp
 * @brief This file contains the declaration of functions related to interfacing
 * between the server and the database.
 */

#include <string>

#include "database.hpp"
#include "server.hpp"
#include "shared/protocol.hpp"

/**
 * @brief Thrown when an viable handler is not found.
 */
class UnknownHandlerException : public std::runtime_error {
   public:
	UnknownHandlerException()
		: std::runtime_error("[Error] An unrecoverable exception occured.") {}
};

// Classes responsible for communicating the requests with the database.
class LoginRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	LoginRequest() : RequestHandler(CODE_LOGIN_USER) {}
};

class OpenAuctionRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	OpenAuctionRequest() : RequestHandler(CODE_OPEN_AUC_CLIENT) {}
};

class CloseAuctionRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	CloseAuctionRequest() : RequestHandler(CODE_CLOSE_AUC_CLIENT) {}
};

class ListStartedAuctionsRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	ListStartedAuctionsRequest() : RequestHandler(CODE_LIST_AUC_USER) {}
};

class ListBiddedAuctionsRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	ListBiddedAuctionsRequest() : RequestHandler(CODE_LIST_MYB_USER) {}
};

class ListAllAuctionsRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	ListAllAuctionsRequest() : RequestHandler(CODE_LIST_ALLAUC_USER) {}
};

class ShowAssetRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	ShowAssetRequest() : RequestHandler(CODE_SHOW_ASSET_CLIENT) {}
};

class BidRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	BidRequest() : RequestHandler(CODE_BID_CLIENT) {}
};

class ShowRecordRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	ShowRecordRequest() : RequestHandler(CODE_SHOWREC_USER) {}
};

class LogoutRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	LogoutRequest() : RequestHandler(CODE_LOGOUT_USER) {}
};

class UnregisterRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	UnregisterRequest() : RequestHandler(CODE_UNREGISTER_USER) {}
};

class WrongRequestUDP : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	WrongRequestUDP() : RequestHandler(CODE_ERROR) {}
};

class WrongRequestTCP : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	WrongRequestTCP() : RequestHandler(CODE_ERROR) {}
};

#endif