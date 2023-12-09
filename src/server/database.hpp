#ifndef __DATABASE__
#define __DATABASE__

#include <dirent.h>
#include <errno.h>
#include <stdint.h>
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

class Database {};

typedef struct {
	std::string user_id;
	std::string name;
	std::string asset_fname;
	std::string start_value;
	std::string timeactive;
	std::string current_date;
	uint32_t current_time;
	bool valid = true;
} Start;

typedef struct {
	std::string user_id;
	std::string value;
	std::string current_date;
	uint32_t time_passed;
	bool valid = true;
} Bid;

int CheckUserExisted(const char *user_id_dirname);
int CreateUserDir(std::string user_id);
int CreateAuctionDir(std::string a_id);
int CreateLogin(std::string user_id);
int CreatePassword(std::string user_id, std::string password);
uint32_t RegisterHost(std::string user_id, std::string a_id);
uint32_t RegisterBid(std::string user_id, std::string a_id);
uint32_t CheckLogoutExists(const char *login_id_fname);
uint32_t EraseLogin(std::string user_id);
uint32_t CheckPasswordExists(const char *password_fname);
uint32_t ErasePassword(std::string user_id);
uint32_t CheckAssetFile(std::string asset_fname);
uint32_t CreateStartFile(std::string a_id, std::string user_id,
                         std::string name, std::string asset_fname,
                         std::string start_value, std::string timeactive);
uint32_t CheckEndExists(const char *end_fname);
uint32_t CreateEndFile(std::string a_id);
uint32_t CreateAssetFile(std::string a_id, std::string asset_fname);
uint32_t CreateBidFile(std::string a_id, std::string user_id,
                       std::string value);
Start GetStart(std::string a_id);
uint32_t GetStartTime(std::string a_id);
std::string GetStartTimeactive(std::string a_id);
std::string GetStartName(std::string a_id);
std::string GetStartValue(std::string a_id);
Bid GetBid(std::string bid_fname);
std::string GetBidName(std::string bid_fname);
std::string GetBidValue(std::string bid_fname);
std::string GetBidDate(std::string bid_fname);
char GetCurrentDate();
uint32_t UserLoggedIn(std::string user_id);
uint32_t UserRegistered(std::string user_id);
uint32_t CorrectPassword(std::string user_id, std::string password);
std::string GetAssetFname(std::string a_id);
std::stringstream GetAssetData(std::string a_id, std::string asset_fname);
uint32_t CreateBaseDir();
uint32_t LoginUser(std::string user_id, std::string password);
uint32_t Logout(std::string user_id);
uint32_t Unregister(std::string user_id);
uint32_t Open(std::string user_id, std::string name, std::string asset_fname,
              std::string start_value, std::string timeactive);
uint32_t Close(std::string a_id);
std::string ShowRecord(std::string a_id);
#endif