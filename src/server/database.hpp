#ifndef __DATABASE__
#define __DATABASE__

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <algorithm>
#include <condition_variable>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#define DB_LOGIN_NOK      -1
#define DB_LOGIN_OK       0
#define DB_LOGIN_REGISTER 2

#define DB_LOGOUT_NOK          -1
#define DB_LOGOUT_OK           0
#define DB_LOGOUT_UNREGISTERED 2

#define DB_UNREGISTER_NOK     -1
#define DB_UNREGISTER_OK      0
#define DB_UNREGISTER_UNKNOWN 2

#define DB_CLOSE_NOK           -1
#define DB_CLOSE_OK            0
#define DB_CLOSE_ENDED_ALREADY 2

#define DB_OPEN_NOT_LOGGED_IN -1
#define DB_OPEN_CREATE_FAIL   -2

#define DB_AUCTION_UNFINISHED -1

#define DB_SHOW_ASSET_ERROR asset

#define DB_BID_NOK    -2
#define DB_BID_REFUSE -1
#define DB_BID_ACCEPT 0

class AuctionNotFound : public std::runtime_error {
   public:
	AuctionNotFound() : std::runtime_error("[ERROR] Couldn't find auction.") {}
};

class UserNotLoggedIn : public std::runtime_error {
   public:
	UserNotLoggedIn()
		: std::runtime_error(
			  "[ERROR] User can't perform this action unlogged.") {}
};

class UserDoesNotExist : public std::runtime_error {
   public:
	UserDoesNotExist() : std::runtime_error("[ERROR] User does not exist.") {}
};

class IncorrectPassword : public std::runtime_error {
   public:
	IncorrectPassword() : std::runtime_error("[ERROR] Incorrect password.") {}
};

class AuctionNotOwnedByUser : public std::runtime_error {
   public:
	AuctionNotOwnedByUser()
		: std::runtime_error("[ERROR] User doesn't own the auction.") {}
};

class AuctionAlreadyClosed : public std::runtime_error {
   public:
	AuctionAlreadyClosed()
		: std::runtime_error("[ERROR] Auction is already closed.") {}
};

class AssetDoesNotExist : public std::runtime_error {
   public:
	AssetDoesNotExist() : std::runtime_error("[ERROR] Asset does not exist.") {}
};

class LargerBidAlreadyExists : public std::runtime_error {
   public:
	LargerBidAlreadyExists()
		: std::runtime_error("[ERROR] Larger bid already exists.") {}
};

class BidOnSelf : public std::runtime_error {
   public:
	BidOnSelf() : std::runtime_error("[ERROR] User can't bid on self.") {}
};

typedef struct {
	std::string user_id;
	std::string name;
	std::string asset_fname;
	std::string start_value;
	std::string timeactive;
	std::string current_date;
	uint32_t current_time;
} StartInfo;

typedef struct {
	std::string end_date;
	uint32_t end_time;
} EndInfo;

typedef struct {
	std::string user_id;
	std::string value;
	std::string current_date;
	uint32_t time_passed;
} BidInfo;

typedef struct {
	std::string asset_fname;
	size_t fsize;
	std::string fdata;
} AssetInfo;

typedef struct {
	std::string a_id;
	bool active = false;
} AuctionListing;

typedef std::vector<AuctionListing> AuctionList;
typedef std::vector<BidInfo> BidList;

typedef struct {
	std::string host_id;
	std::string auction_name;
	std::string asset_fname;
	std::string start_value;
	std::string start_datetime;
	std::string timeactive;
	BidList list;
	bool active = false;
	std::string end_datetime;
	uint32_t end_timeelapsed;
} AuctionRecord;

bool CompareByAid(const AuctionListing &a, const AuctionListing &b);

class semaphore {
	std::mutex mutex_;
	std::condition_variable condition_;
	unsigned long count_ = 0;  // Initialized as locked.

   public:
	void release() {
		std::lock_guard<decltype(mutex_)> lock(mutex_);
		++count_;
		condition_.notify_one();
	}

	void acquire() {
		std::unique_lock<decltype(mutex_)> lock(mutex_);
		while (!count_)  // Handle spurious wake-ups.
			condition_.wait(lock);
		--count_;
	}
};

class Database {
   protected:
	int CheckUserExisted(const char *user_id_dirname);
	int CheckUserRegistered(std::string user_id);
	int CreateUserDir(std::string user_id);
	int CreateAuctionDir(std::string a_id);
	int CreateLogin(std::string user_id);
	int CreatePassword(std::string user_id, std::string password);
	int RegisterHost(std::string user_id, std::string a_id);
	int RegisterBid(std::string user_id, std::string a_id);
	int CheckLoginExists(const char *login_id_fname);
	int EraseLogin(std::string user_id);
	int CheckPasswordExists(const char *password_fname);
	int ErasePassword(std::string user_id);
	int CheckAssetFile(std::string asset_fname);
	int CreateStartFile(std::string a_id, std::string user_id, std::string name,
	                    std::string asset_fname, std::string start_value,
	                    std::string timeactive);
	int CheckEndExists(const char *end_fname);
	int CreateEndFile(std::string a_id);
	int CreateAssetFile(std::string a_id, std::string asset_fname,
	                    std::string data);
	int CreateBidFile(std::string a_id, std::string user_id, std::string value);
	int GetStart(std::string a_id, StartInfo &result);
	int GetEnd(const char *end_fname, EndInfo &end);
	int GetBid(std::string bid_fname, BidInfo &result);
	std::string GetCurrentDate();
	int CorrectPassword(std::string user_id, std::string password);
	std::string GetAssetDir(std::string a_id);
	int CheckAuctionExists(std::string a_id);
	int CheckAuctionBelongs(std::string a_id, std::string user_id);
	std::string GetAssetData(std::string asset_fname);
	int Close(std::string a_id);

   public:
	int CreateBaseDir();
	int CheckUserLoggedIn(std::string user_id);
	int LoginUser(std::string user_id, std::string password);
	int Logout(std::string user_id, std::string password);
	int Unregister(std::string user_id, std::string password);
	int Open(std::string user_id, std::string name, std::string password,
	         std::string asset_fname, std::string start_value,
	         std::string timeactive, size_t fsize, std::string data);
	int CloseAuction(std::string a_id, std::string user_id,
	                 std::string password);
	AuctionList MyAuctions(std::string user_id);
	AuctionList MyBids(std::string user_id);
	AuctionList List();
	AssetInfo ShowAsset(std::string a_id);
	int Bid(std::string user_id, std::string password, std::string a_id,
	        std::string value);
	AuctionRecord ShowRecord(std::string a_id);
};

#endif