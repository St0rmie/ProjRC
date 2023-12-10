#ifndef __DATABASE__
#define __DATABASE__

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

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

typedef struct {
	std::string user_id;
	std::string name;
	std::string asset_fname;
	std::string start_value;
	std::string timeactive;
	std::string current_date;
	uint32_t current_time;
} Start;

typedef struct {
	std::string user_id;
	std::string value;
	std::string current_date;
	uint32_t time_passed;
} Bid;

class Database {
   protected:
	int CheckUserExisted(const char *user_id_dirname);
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
	int CreateAssetFile(std::string a_id, std::string asset_fname);
	int CreateBidFile(std::string a_id, std::string user_id, std::string value);
	int GetStart(std::string a_id, Start &result);
	int GetBid(std::string bid_fname, Bid &result);
	std::string GetCurrentDate();
	int UserLoggedIn(std::string user_id);
	int UserRegistered(std::string user_id);
	int CorrectPassword(std::string user_id, std::string password);
	std::string GetAssetFname(std::string a_id);
	std::stringstream GetAssetData(std::string a_id, std::string asset_fname);

   public:
	int CreateBaseDir();
	int LoginUser(std::string user_id, std::string password);
	int Logout(std::string user_id, std::string password);
	int Unregister(std::string user_id, std::string password);
	int Open(std::string user_id, std::string name, std::string asset_fname,
	         std::string start_value, std::string timeactive);
	int Close(std::string a_id);
	std::string ShowRecord(std::string a_id);
};

#endif