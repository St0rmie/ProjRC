#ifndef __DATABASE__
#define __DATABASE__

/**
 * @file database.hpp
 * @brief This file contains the declaration of functions related to managing
 * the database.
 */

#include <dirent.h>
#include <errno.h>
#include <semaphore.h>
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

/**
 * @brief Thrown when there's a failure in the semaphore.
 */
class SemException : public std::runtime_error {
   public:
	SemException() : std::runtime_error("[ERROR] Error in semaphore.") {}
};

/**
 * @brief Thrown when the auction isn't found.
 */
class AuctionNotFound : public std::runtime_error {
   public:
	AuctionNotFound() : std::runtime_error("[ERROR] Couldn't find auction.") {}
};

/**
 * @brief Thrown when the user isn't logged in for an action that needs it.
 */
class UserNotLoggedIn : public std::runtime_error {
   public:
	UserNotLoggedIn()
		: std::runtime_error(
			  "[ERROR] User can't perform this action unlogged.") {}
};

/**
 * @brief Thrown when the user doesn't exist for an action that need them.
 */
class UserDoesNotExist : public std::runtime_error {
   public:
	UserDoesNotExist() : std::runtime_error("[ERROR] User does not exist.") {}
};

/**
 * @brief Thrown when the password is incorrect.
 */
class IncorrectPassword : public std::runtime_error {
   public:
	IncorrectPassword() : std::runtime_error("[ERROR] Incorrect password.") {}
};

/**
 * @brief Thrown when the user attempts to perform an action that can't be done
 * on an auction that isn't theirs.
 */
class AuctionNotOwnedByUser : public std::runtime_error {
   public:
	AuctionNotOwnedByUser()
		: std::runtime_error("[ERROR] User doesn't own the auction.") {}
};

/**
 * @brief Thrown when the auction is already closed for an action that needs an
 * open one.
 */
class AuctionAlreadyClosed : public std::runtime_error {
   public:
	AuctionAlreadyClosed()
		: std::runtime_error("[ERROR] Auction is already closed.") {}
};

/**
 * @brief Thrown when the asset doesn't exist for an action that needs it.
 */
class AssetDoesNotExist : public std::runtime_error {
   public:
	AssetDoesNotExist() : std::runtime_error("[ERROR] Asset does not exist.") {}
};

/**
 * @brief Thrown when a bid is attempted on an auction that already has a larger
 * one.
 */
class LargerBidAlreadyExists : public std::runtime_error {
   public:
	LargerBidAlreadyExists()
		: std::runtime_error("[ERROR] Larger bid already exists.") {}
};

/**
 * @brief Thrown when a user attempts to bid on an auction they created.
 */
class BidOnSelf : public std::runtime_error {
   public:
	BidOnSelf() : std::runtime_error("[ERROR] User can't bid on self.") {}
};

/**
 * @brief A struct containing the information of the start file.
 */
typedef struct {
	std::string user_id;
	std::string name;
	std::string asset_fname;
	std::string start_value;
	std::string timeactive;
	std::string current_date;
	uint32_t current_time;
} StartInfo;

/**
 * @brief A struct containing the information of the end file.
 */
typedef struct {
	std::string end_date;
	uint32_t end_time;
} EndInfo;

/**
 * @brief A struct containing the information of the bid.
 */
typedef struct {
	std::string user_id;
	std::string value;
	std::string current_date;
	uint32_t time_passed;
} BidInfo;

/**
 * @brief A struct containing the information of the asset.
 */
typedef struct {
	std::string asset_fname;
	size_t fsize;
	std::string fdata;
} AssetInfo;

/**
 * @brief A struct containing the auction's id and whether it's still active.
 */
typedef struct {
	std::string a_id;
	bool active = false;
} AuctionListing;

typedef std::vector<AuctionListing> AuctionList;
typedef std::vector<BidInfo> BidList;

/**
 * @brief A struct containing the information of the auction, including the bids
 * places and when it ended.
 */
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
bool CompareByValue(const BidInfo &a, const BidInfo &b);

/**
 * @brief  Class that represents a database instance. Contains all the functions
 * necessary to create new files and interact with the existing ones in order to
 * simulate the users and auctions.
 */
class Database {
   protected:
	sem_t *_sem;
	int _lock_id;

	// Internal functions
	int semaphore_init(int port_n);
	void semaphore_wait();
	void semaphore_post();
	int semaphore_destroy();
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
	int CreateBaseDir(int sem_id);
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