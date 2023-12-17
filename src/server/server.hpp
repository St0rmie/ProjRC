#ifndef __SERVER__
#define __SERVER__

#include <netdb.h>

#include <unordered_map>

#include "database.hpp"
#include "shared/protocol.hpp"
#include "shared/utils.hpp"

#define EXCEPTION_RETRY_MAX 5

class UnrecoverableException : public std::runtime_error {
   public:
	UnrecoverableException(std::string message) : std::runtime_error(message) {}
};

class Address {
   public:
	int socket;
	struct sockaddr_in addr;
	socklen_t size;
};

class Server {
	std::string _hostname = DEFAULT_HOSTNAME;
	std::string _port = DEFAULT_PORT;

	void resolveServerAddress(std::string& hostname, std::string& port);
	void sendUdpMessage(ProtocolMessage& message);
	void waitForUdpMessage(ProtocolMessage& message);
	void openTcpSocket();
	void sendTcpMessage(ProtocolMessage& message);
	void waitForTcpMessage(ProtocolMessage& message);
	void closeTcpSocket();
	void setupSignalHandlers();
	void configServer(int argc, char* argv[]);
	void setup_sockets();
	void resolveServerAddress(std::string& port);

   public:
	int _udp_socket_fd = -1;
	int _tcp_socket_fd = -1;
	struct addrinfo* _server_udp_addr = NULL;
	struct addrinfo* _server_tcp_addr = NULL;
	Database _database;
	bool _verbose = false;
	Server(int argc, char* argv[]);
	~Server();
	void sendUdpMessage(ProtocolMessage& out_message, Address& addr_from);
	void sendTcpMessage();
};

class RequestHandler {
   protected:
	RequestHandler(const char* protocol_code) : _protocol_code{protocol_code} {}

   public:
	const char* _protocol_code;
	virtual void handle(MessageAdapter& message, Server& server,
	                    Address& address) = 0;
};

class RequestManager {
   private:
	std::unordered_map<std::string, std::shared_ptr<RequestHandler>>
		_udp_handlers;
	std::unordered_map<std::string, std::shared_ptr<RequestHandler>>
		_tcp_handlers;

   public:
	void registerRequestHandlers();
	void registerRequest(std::shared_ptr<RequestHandler> handler, int type);
	void callHandlerRequest(MessageAdapter& message, Server& client,
	                        Address& address, int type);
};

void processUDP(Server& server, RequestManager& manager);
void processTCPChild(Server& server, RequestManager& manager, Address addr_from,
                     int connection_fd);
void processTCP(Server& server, RequestManager& manager);
void wait_for_udp_message(Server& server, RequestManager& manager);

#endif