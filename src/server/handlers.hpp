/**
 * @file handlers.hpp
 * @brief This file contains the declaration of request handlers for the server.
 * The request handlers are responsible for handling specific types of requests
 * received by the server and performing the necessary actions including calling
 * the database.
 */

#ifndef __SERVER_HANDLERS__
#define __SERVER_HANDLERS__

#include <string>

#include "database.hpp"
#include "server.hpp"
#include "shared/protocol.hpp"

/**
 * @brief Thrown when a viable handler is not found.
 */
class UnknownHandlerException : public std::runtime_error {
   public:
	/**
	 * @brief Constructs an UnknownHandlerException object.
	 */
	UnknownHandlerException()
		: std::runtime_error("[Error] An unrecoverable exception occurred.") {}
};

// -----------------------------------
// | Request Handlers                |
// -----------------------------------

/**
 * @brief Login request handler.
 * This handler is responsible for handling the login request from the client.
 */
class LoginRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	/**
	 * @brief Constructs a LoginRequest object.
	 */
	LoginRequest() : RequestHandler(CODE_LOGIN_USER) {}
};

/**
 * @brief Open Auction request handler.
 * This handler is responsible for handling the request to open an auction.
 */
class OpenAuctionRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	/**
	 * @brief Constructs an OpenAuctionRequest object.
	 */
	OpenAuctionRequest() : RequestHandler(CODE_OPEN_AUC_CLIENT) {}
};

/**
 * @brief Close Auction request handler.
 * This handler is responsible for handling the request to close an auction.
 */
class CloseAuctionRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	/**
	 * @brief Constructs a CloseAuctionRequest object.
	 */
	CloseAuctionRequest() : RequestHandler(CODE_CLOSE_AUC_CLIENT) {}
};

/**
 * @brief List My Auctions request handler.
 * This handler is responsible for handling the request to list the auctions of
 * the current user.
 */
class ListStartedAuctionsRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	/**
	 * @brief Constructs a ListStartedAuctionsRequest object.
	 */
	ListStartedAuctionsRequest() : RequestHandler(CODE_LIST_AUC_USER) {}
};

/**
 * @brief List My Bids request handler.
 * This handler is responsible for handling the request to list the auctions
 * bidded by the current user.
 */
class ListBiddedAuctionsRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	/**
	 * @brief Constructs a ListBiddedAuctionsRequest object.
	 */
	ListBiddedAuctionsRequest() : RequestHandler(CODE_LIST_MYB_USER) {}
};

/**
 * @brief List All Auctions request handler.
 * This handler is responsible for handling the request to list all the
 * auctions.
 */
class ListAllAuctionsRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	/**
	 * @brief Constructs a ListAllAuctionsRequest object.
	 */
	ListAllAuctionsRequest() : RequestHandler(CODE_LIST_ALLAUC_USER) {}
};

/**
 * @brief Show Asset request handler.
 * This handler is responsible for handling the request to show the asset.
 */
class ShowAssetRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	/**
	 * @brief Constructs a ShowAssetRequest object.
	 */
	ShowAssetRequest() : RequestHandler(CODE_SHOW_ASSET_CLIENT) {}
};

/**
 * @brief Bid request handler.
 * This handler is responsible for handling the bid request from the client.
 */
class BidRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	/**
	 * @brief Constructs a BidRequest object.
	 */
	BidRequest() : RequestHandler(CODE_BID_CLIENT) {}
};

/**
 * @brief Show record request handler.
 * This handler is responsible for handling the request to show the record of an
 * auction.
 */
class ShowRecordRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	/**
	 * @brief Constructs a ShowRecordRequest object.
	 */
	ShowRecordRequest() : RequestHandler(CODE_SHOWREC_USER) {}
};

/**
 * @brief Logout request handler.
 * This handler is responsible for handling the logout request from the client.
 */
class LogoutRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	/**
	 * @brief Constructs a LogoutRequest object.
	 */
	LogoutRequest() : RequestHandler(CODE_LOGOUT_USER) {}
};

/**
 * @brief Unregister request handler.
 * This handler is responsible for handling the request to unregister the
 * current user.
 */
class UnregisterRequest : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	/**
	 * @brief Constructs an UnregisterRequest object.
	 */
	UnregisterRequest() : RequestHandler(CODE_UNREGISTER_USER) {}
};

/**
 * @brief Wrong request handler (UDP).
 * This handler is responsible for handling the wrong request received over UDP.
 */
class WrongRequestUDP : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	/**
	 * @brief Constructs a WrongRequestUDP object.
	 */
	WrongRequestUDP() : RequestHandler(CODE_ERROR) {}
};

/**
 * @brief Wrong request handler (TCP).
 * This handler is responsible for handling the wrong request received over TCP.
 */
class WrongRequestTCP : public RequestHandler {
	virtual void handle(MessageAdapter &message, Server &client,
	                    Address &address);

   public:
	/**
	 * @brief Constructs a WrongRequestTCP object.
	 */
	WrongRequestTCP() : RequestHandler(CODE_ERROR) {}
};

#endif