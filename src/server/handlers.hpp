#ifndef __SERVER_HANDLERS__
#define __ SERVER_HANDLERS__

#include <string>
#include <unordered_map>

#include "database.hpp"
#include "server.hpp"

// Thrown when an viable handler is not found
class UnknownHandlerException : public std::runtime_error {
   public:
	UnknownHandlerException()
		: std::runtime_error("[Error] An unrecoverable exception occured.") {}
};

class RequestHandler {
   protected:
	RequestHandler(const char *protocol_code) : _protocol_code{protocol_code} {}

   public:
	const char *_protocol_code;
	virtual void handle(MessageAdapter &message, Server &server,
	                    Address &address) = 0;
};

class RequestManager {
   private:
	std::unordered_map<std::string, std::shared_ptr<RequestHandler>>
		_udp_handlers;
	std::unordered_map<std::string, std::shared_ptr<RequestHandler>>
		_tcp_handlers;

   public:
	void registerRequestHandlers(RequestManager &manager);
	void registerRequest(std::shared_ptr<RequestHandler> handler, int type);
	void callHandlerRequest(MessageAdapter &message, Server &client,
	                        Address &address, int type);
};

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

void registerRequestHandlers(RequestManager &manager);

#endif