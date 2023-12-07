#ifndef __SERVER_HANDLERS__
#define __ SERVER_HANDLERS__

#include <string>
#include <unordered_map>

#include "server.hpp"
#include "database.hpp"

class RequestHandler {
   protected:
	RequestHandler(const char *protocol_code)
		: _protocol_code{protocol_code} {}

   public:
	const char *_protocol_code;
	virtual void handle(std::string _protocol_code, Server &server) = 0;
};

class RequestManager{
    private:
	std::unordered_map<std::string, std::shared_ptr<RequestHandler>> handlers;

   public:
    void registerRequestHandlers(RequestManager &manager);
	void registerRequest(std::shared_ptr<RequestHandler> handler);
	void waitRequest(Server &client);
};

class LoginRequest : public RequestHandler {
	virtual void handle(std::string name, Server &server);

   public:
	LoginRequest()
		: RequestHandler(CODE_LOGIN_USER) {}
};

class OpenAuctionRequest : public RequestHandler {
	virtual void handle(std::string name, Server &server);

   public:
	OpenAuctionRequest()
		: RequestHandler(CODE_OPEN_AUC_CLIENT) {}
};

class CloseAuctionRequest : public RequestHandler {
	virtual void handle(std::string name, Server &server);

   public:
	CloseAuctionRequest()
		: RequestHandler(CODE_CLOSE_AUC_CLIENT) {}
};

class ListStartedAuctionsRequest : public RequestHandler {
	virtual void handle(std::string name, Server &server);

   public:
	ListStartedAuctionsRequest()
		: RequestHandler(CODE_LIST_AUC_USER) {}
};

#endif