#ifndef __PROTOCOL__
#define __PROTOCOL__

#include <sstream>
#include <string>

class UdpPacket {
   public:
	virtual std::stringstream construct() = 0;
	virtual void deconstruct(std::stringstream &buffer) = 0;
};

class TcpPacket {
   public:
	virtual void send(int fd) = 0;
	virtual void receive(int fd) = 0;
};

// -----------------------------------
// | Client packets for each command |
// -----------------------------------

// Login (LIN) -> UDP
class ClientLoginUser : public UdpPacket {
   public:
	uint32_t user_id;
	std::string password;

	std::stringstream construct();
	void deconstruct(std::stringstream &buffer);
};

// Close Auction (CLS) -> TCP
class ClientCloseAuction : public TcpPacket {
   public:
	uint32_t user_id;
	std::string password;
	uint32_t auction_id;

	void send(int fd);
	void receive(int fd);
};

// List started auctions by a specified user (LMA)
class ClientListStartedAuctions : public UdpPacket {
   public:
	uint32_t user_id;

	std::stringstream construct();
	void deconstruct(std::stringstream &buffer);
};

// List bidded auctions by a specified suer (LMB)
class ClientListBiddedAuction : public UdpPacket {
   public:
	uint32_t user_id;

	std::stringstream construct();
	void deconstruct(std::stringstream &buffer);
};

// List all auctions on the system (LST)
class ClientListAllAuctions : public UdpPacket {
   public:
	std::stringstream construct();
	void deconstruct(std::stringstream &buffer);
};

// Show asset of a specified auction (SAS)
class ClientShowAsset : public TcpPacket {
   public:
	uint32_t auction_id;

	void send(int fd);
	void receive(int fd);
};

// A user bids on an auction (BID)
class ClientBid : public TcpPacket {
   public:
	uint32_t user_id;
	std::string password;
	uint32_t auction_id;
	uint32_t value;

	void send(int fd);
	void receive(int fd);
};

class ClientShowRecord : public UdpPacket {};
class ClientLogout : public UdpPacket {};
class ClientUnregister : public UdpPacket {};

// Server packets for each command
class ServerCreateAuction : public UdpPacket {};
class ServerCloseAuction : public UdpPacket {};
class ServerListStartedAuction : public UdpPacket {};
class ServerListBiddedAuction : public UdpPacket {};
class ServerListAllAuctions : public UdpPacket {};
class ServerShowAsset : public UdpPacket {};
class ServerBid : public UdpPacket {};
class ServerShowRecord : public UdpPacket {};
class ServerLogout : public UdpPacket {};
class ServerUnregister : public UdpPacket {};

#endif __PROTOCOL__