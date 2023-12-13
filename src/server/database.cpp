#include "database.hpp"

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

#include "shared/utils.hpp"
#include "shared/verifications.hpp"

namespace fs = std::filesystem;

int Database::CheckUserExisted(const char *user_id_dirname) {
	std::cout << user_id_dirname << std::endl;
	DIR *dir = opendir(user_id_dirname);

	if (dir) {
		closedir(dir);
		return 0;
	} else if (ENOENT == errno) {
		return -1;
	}

	return -1;
}

/* -1 error, 0 if user is registered in, 1 if not*/
int Database::CheckUserRegistered(std::string user_id) {
	if (verify_user_id(user_id) == -1) {
		return -1;
	}

	std::string pass_fname = "ASDIR/USERS/" + user_id;
	pass_fname += "/";
	pass_fname += user_id;
	pass_fname += "_pass.txt";

	FILE *fp;

	const char *pass_dir = pass_fname.c_str();

	fp = fopen(pass_dir, "r");
	if (fp == NULL) {
		return -1;
	}

	return 0;
}

/* -1 error, 0 if user is logged in, 1 if not*/
int Database::CheckUserLoggedIn(std::string user_id) {
	if (verify_user_id(user_id) == -1) {
		return -1;
	}

	std::string login_fname = "ASDIR/USERS/" + user_id;
	login_fname += "/";
	login_fname += user_id;
	login_fname += "_login.txt";

	FILE *fp;

	const char *login_dir = login_fname.c_str();

	fp = fopen(login_dir, "r");
	if (fp == NULL) {
		return -1;
	}

	return 0;
}

/* Returns -1 if failed, 0 if sucessful, 2 if user already existed*/
int Database::CreateUserDir(std::string user_id) {
	if (verify_user_id(user_id) == -1) {
		return -1;
	}

	std::string user_id_dir = "ASDIR/USERS/" + user_id;

	const char *user_id_dirname = user_id_dir.c_str();

	if (CheckUserExisted(user_id_dirname) == 0) {
		return 2;
	}

	std::string hosted_dir = "ASDIR/USERS/" + user_id;
	hosted_dir += "/HOSTED";
	std::string bidded_dir = "ASDIR/USERS/" + user_id;
	bidded_dir += "/BIDDED";

	const char *hosted_dirname = hosted_dir.c_str();
	const char *bidded_dirname = bidded_dir.c_str();

	if (mkdir(user_id_dirname, 0700) == -1) {
		return -1;
	}

	if (mkdir(hosted_dirname, 0700) == -1) {
		return -1;
	}

	if (mkdir(bidded_dirname, 0700) == -1) {
		return -1;
	}

	return 0;
}

int Database::CreateAuctionDir(std::string a_id) {
	if (verify_auction_id(a_id) == -1) {
		std::cout << "A1" << std::endl;
		return -1;
	}

	std::string a_id_dir = "ASDIR/AUCTIONS/" + a_id;
	std::string bid_dir = "ASDIR/AUCTIONS/" + a_id;
	bid_dir += "/BIDS";
	std::string asset_dir = "ASDIR/AUCTIONS/" + a_id;
	asset_dir += "/ASSET";

	const char *a_id_dirname = a_id_dir.c_str();
	const char *bid_dirname = bid_dir.c_str();
	const char *asset_dirname = asset_dir.c_str();

	if (mkdir(a_id_dirname, 0700) == -1) {
		std::cout << a_id_dirname << std::endl;
		return -1;
	}
	if (mkdir(bid_dirname, 0700) == -1) {
		std::cout << "A3" << std::endl;
		return -1;
	}
	if (mkdir(asset_dirname, 0700) == -1) {
		std::cout << "A4" << std::endl;
		return -1;
	}

	return 0;
}

int Database::CreateLogin(std::string user_id) {
	if (verify_user_id(user_id) == -1) {
		return -1;
	}

	FILE *fp;

	std::string user_id_name = "ASDIR/USERS/" + user_id;
	user_id_name += "/";
	user_id_name += user_id;
	user_id_name += "_login.txt";

	const char *user_id_fname = user_id_name.c_str();

	fp = fopen(user_id_fname, "w");
	if (fp == NULL) {
		return -1;
	}

	fclose(fp);
	return 0;
}

int Database::CreatePassword(std::string user_id, std::string password) {
	if (verify_password(password) == -1) {
		return -1;
	}

	std::string password_name = "ASDIR/USERS/" + user_id;
	password_name += "/";
	password_name += user_id;
	password_name += "_pass.txt";

	const char *password_fname = password_name.c_str();
	const char *password_file = password.c_str();

	FILE *fp = fopen(password_fname, "w");
	if (fp == NULL) {
		return -1;
	}

	fprintf(fp, "%s", password_file);

	fclose(fp);
	return 0;
}

int Database::RegisterHost(std::string user_id, std::string a_id) {
	if (verify_user_id(user_id) == -1) {
		return -1;
	}

	if (verify_auction_id(a_id) == -1) {
		return -1;
	}

	FILE *fp;

	std::string host_name = "ASDIR/USERS/" + user_id;
	host_name += "/HOSTED/";
	host_name += a_id;
	host_name += ".txt";

	const char *host_fname = host_name.c_str();

	fp = fopen(host_fname, "w");
	if (fp == NULL) {
		return -1;
	}
	fclose(fp);
	return 0;
}

int Database::RegisterBid(std::string user_id, std::string a_id) {
	if (verify_user_id(user_id) == -1) {
		return -1;
	}

	if (verify_auction_id(a_id) == -1) {
		return -1;
	}

	FILE *fp;

	std::string bid_name = "ASDIR/USERS/" + user_id;
	bid_name += "/BIDDED/";
	bid_name += a_id;
	bid_name += ".txt";

	const char *bid_fname = bid_name.c_str();

	fp = fopen(bid_fname, "w");
	if (fp == NULL) {
		return -1;
	}
	fclose(fp);
	return 0;
}

/* Returns -1 if logout doesn't exist, 0 if it does*/
int Database::CheckLoginExists(const char *login_id_fname) {
	if (access(login_id_fname, F_OK) == 0) {
		return 0;
	} else {
		return -1;
	}
}

/* Returns -1 if failed, 0 if sucessful, 2 if user is logged out*/
int Database::EraseLogin(std::string user_id) {
	if (verify_user_id(user_id) == -1) {
		return -1;
	}

	std::string user_id_dir = "ASDIR/USERS/" + user_id;

	const char *user_id_dirname = user_id_dir.c_str();

	if (CheckUserExisted(user_id_dirname) == -1) {
		return -1;
	}

	std::string login_id_name = "ASDIR/USERS/" + user_id;
	login_id_name += "/";
	login_id_name += user_id;
	login_id_name += "_login.txt";

	const char *login_id_fname = login_id_name.c_str();

	if (CheckLoginExists(login_id_fname) == -1) {
		return 2;
	}

	unlink(login_id_fname);

	return 0;
}

/* Returns -1 if password doesn't exist, 0 if it does*/
int Database::CheckPasswordExists(const char *password_fname) {
	if (access(password_fname, F_OK) == 0) {
		return 0;
	} else {
		return -1;
	}
}

/* Returns -1 if failed, 0 if sucessful, 2 if unknown user*/
int Database::ErasePassword(std::string user_id) {
	if (verify_user_id(user_id) == -1) {
		return -1;
	}

	std::string user_id_dir = "ASDIR/USERS/" + user_id;

	const char *user_id_dirname = user_id_dir.c_str();

	if (CheckUserExisted(user_id_dirname) == -1) {
		return 2;
	}

	std::string password_name = "ASDIR/USERS/" + user_id;
	password_name += "/";
	password_name += user_id;
	password_name += "_pass.txt";

	const char *password_fname = password_name.c_str();

	if (CheckPasswordExists(password_fname) == -1) {
		return -1;
	}

	unlink(password_fname);

	return 0;
}

int Database::CreateStartFile(std::string a_id, std::string user_id,
                              std::string name, std::string asset_fname,
                              std::string start_value, std::string timeactive) {
	if (verify_timeactive(timeactive) == -1) {
		return -1;
	}

	if (verify_start_value(start_value) == -1) {
		return -1;
	}

	if (verify_name(name) == -1) {
		return -1;
	}

	if (verify_user_id(user_id) == -1) {
		return -1;
	}

	if (verify_auction_id(a_id) == -1) {
		return -1;
	}

	FILE *fp;
	time_t fulltime;
	std::string current_date = GetCurrentDate();
	time_t current_time = time(&fulltime);

	std::string dir_name = "ASDIR/AUCTIONS/" + a_id;
	dir_name += "/START_";
	dir_name += a_id;
	dir_name += ".txt";

	std::string content = user_id + " ";
	content += name;
	content += " ";
	content += asset_fname;
	content += " ";
	content += start_value;
	content += " ";
	content += timeactive;
	content += " ";
	content += current_date;
	content += " ";
	content += std::to_string(current_time);

	const char *file_content = content.c_str();

	const char *dir_fname = dir_name.c_str();

	fp = fopen(dir_fname, "w");
	if (fp == NULL) {
		return -1;
	}

	fprintf(fp, "%s", file_content);

	fclose(fp);
	return 0;
}

/* Returns -1 if end doesn't exist, 0 if it does*/
int Database::CheckEndExists(const char *end_fname) {
	if (access(end_fname, F_OK) == 0) {
		return 0;
	} else {
		return -1;
	}
}

/* Returns -1 if failed, 0 if sucessful, 2 if already ended*/
int Database::CreateEndFile(std::string a_id) {
	if (verify_auction_id(a_id) == -1) {
		return -1;
	}

	FILE *fp;
	StartInfo start;
	time_t fulltime;
	std::string current_date = GetCurrentDate();
	time_t current_time = time(&fulltime);
	if (GetStart(a_id, start) == -1) {
		throw AuctionNotFound();
		return -1;
	};
	uint32_t start_time = start.current_time;
	uint32_t time_passed = current_time - start_time;
	std::string dir_name = "ASDIR/AUCTIONS/" + a_id;
	dir_name += "/END_";
	dir_name += a_id;
	dir_name += ".txt";

	std::string content = current_date + " ";
	content += std::to_string(time_passed);

	const char *file_content = content.c_str();

	const char *dir_fname = dir_name.c_str();

	if (CheckEndExists(dir_fname) == 0) {
		return 2;
	}

	fp = fopen(dir_fname, "w");
	if (fp == NULL) {
		return -1;
	}

	fprintf(fp, "%s", file_content);

	fclose(fp);

	return 0;
}

int Database::CreateAssetFile(std::string a_id, std::string asset_fname,
                              size_t fsize, std::string data) {
	if (verify_auction_id(a_id) == -1) {
		return -1;
	}

	FILE *fp;

	std::string dir_name = "ASDIR/AUCTIONS/" + a_id;
	dir_name += "/ASSET/";
	dir_name += asset_fname;

	std::ofstream file;
	file.open(dir_name, std::ofstream::trunc);
	if (!file.good()) {
		return -1;
	}

	file << data << std::endl;

	file.close();

	return 0;
}

int Database::CreateBidFile(std::string a_id, std::string user_id,
                            std::string value) {
	if (verify_value(stoi(value)) == -1) {
		return -1;
	}

	if (verify_user_id(user_id) == -1) {
		return -1;
	}

	if (verify_auction_id(a_id) == -1) {
		return -1;
	}
	StartInfo start;
	FILE *fp;
	time_t fulltime;
	std::string current_date = GetCurrentDate();
	time_t current_time = time(&fulltime);
	if (GetStart(a_id, start) == -1) {
		throw AuctionNotFound();
		return -1;
	};
	uint32_t start_time = start.current_time;
	uint32_t time_passed = current_time - start_time;

	std::string dir_name = "ASDIR/AUCTIONS/" + a_id;
	dir_name += "/BIDS/";
	dir_name += value;
	dir_name += ".txt";

	std::string content = user_id + " ";
	content += value;
	content += " ";
	content += current_date;
	content += " ";
	content += std::to_string(time_passed);

	const char *file_content = content.c_str();

	const char *dir_fname = dir_name.c_str();

	fp = fopen(dir_fname, "w");
	if (fp == NULL) {
		return -1;
	}

	fprintf(fp, "%s", file_content);

	fclose(fp);

	return 0;
}

int Database::GetStart(std::string a_id, StartInfo &result) {
	FILE *fp;
	char content[200];

	std::string dir_name = "ASDIR/AUCTIONS/" + a_id;
	dir_name += "/START_";
	dir_name += a_id;
	dir_name += ".txt";

	const char *dir_fname = dir_name.c_str();

	fp = fopen(dir_fname, "r");
	if (fp == NULL) {
		return -1;
	}

	if (fgets(content, 200, fp) == NULL) {
		return -1;
	}

	std::stringstream ss(content);
	std::vector<std::string> parsed_content;
	std::string cont;

	while (ss >> cont) {
		parsed_content.push_back(cont);
	}

	if (parsed_content.size() != 8) {
		return -1;
	}

	result.user_id = parsed_content[0];
	result.name = parsed_content[1];
	result.asset_fname = parsed_content[2];
	result.start_value = parsed_content[3];
	result.timeactive = parsed_content[4];
	result.current_date = parsed_content[5] + " ";
	result.current_date += parsed_content[6];
	result.current_time = stoi(parsed_content[7]);

	fclose(fp);

	return 0;
}

int Database::GetEnd(const char *end_fname, EndInfo &end) {
	FILE *fp;
	char content[200];
	fp = fopen(end_fname, "r");
	if (fp == NULL) {
		std::cout << "shitty fname" << std::endl;
		return -1;
	}
	if (fgets(content, 200, fp) == NULL) {
		std::cout << "dafuq" << std::endl;
		return -1;
	}
	std::stringstream ss(content);
	std::vector<std::string> parsed_content;
	std::string cont;

	while (ss >> cont) {
		parsed_content.push_back(cont);
		std::cout << "shit" << std::endl;
	}

	if (parsed_content.size() != 3) {
		std::cout << "this borking" << std::endl;
		return -1;
	}

	end.end_date = parsed_content[0] + " ";
	end.end_date += parsed_content[1];
	end.end_time = stoi(parsed_content[2]);

	fclose(fp);
	std::cout << "getendends" << std::endl;
	return 0;
}

int Database::GetBid(std::string bid_fname, BidInfo &result) {
	FILE *fp;
	char content[200];

	const char *bif_file_name = bid_fname.c_str();

	fp = fopen(bif_file_name, "r");
	if (fp == NULL) {
		return -1;
	}

	if (fgets(content, 100, fp) == NULL) {
		return -1;
	}

	std::stringstream ss(content);
	std::vector<std::string> parsed_content;
	std::string cont;

	while (ss >> cont) {
		parsed_content.push_back(cont);
	}

	if (parsed_content.size() != 5) {
		return -1;
	}

	result.user_id = parsed_content[0];

	result.user_id = parsed_content[0];
	result.value = parsed_content[1];
	result.current_date = parsed_content[2] + " ";
	result.current_date += parsed_content[3];
	result.time_passed = stoi(parsed_content[4]);

	fclose(fp);

	return 0;
}

std::string Database::GetCurrentDate() {
	time_t fulltime;
	struct tm *current_time;
	char time_str[40];
	time(&fulltime);  // Get current time in seconds starting at 1970
	current_time = gmtime(&fulltime);  // Convert time to YYYY−MM−DD HH:MM:SS
	sprintf(time_str, "%4u-%02u-%02u %02u:%02u:%02u",
	        current_time->tm_year + 1900, current_time->tm_mon + 1,
	        current_time->tm_mday, current_time->tm_hour, current_time->tm_min,
	        current_time->tm_sec);
	std::string str(time_str);
	std::cout << str << std::endl;
	return str;
}

// 1 it is, 0 is not, -1 is error
int Database::CorrectPassword(std::string user_id, std::string password) {
	if (verify_user_id(user_id) == -1) {
		return -1;
	}

	if (verify_password(password) == -1) {
		return -1;
	}

	FILE *fp;

	char content[10];

	std::string pass_name = "ASDIR/USERS/" + user_id;
	pass_name += "/";
	pass_name += user_id;
	pass_name += "_pass.txt";

	const char *pass_fname = pass_name.c_str();

	fp = fopen(pass_fname, "r");
	if (fp == NULL) {
		return -1;
	}

	if (fgets(content, 9, fp) == NULL) {
		return -1;
	}

	fclose(fp);

	if (content == password) {
		return 1;
	} else {
		return 0;
	}
}

int Database::Close(std::string a_id) {
	int ended = CreateEndFile(a_id);

	if (ended == -1) {
		return DB_CLOSE_NOK;  // Error
	}

	if (ended == 0) {
		return DB_CLOSE_OK;  // Auction successfully closed
	}

	if (ended == 2) {
		throw AuctionAlreadyClosed();
		return DB_CLOSE_ENDED_ALREADY;  // Auction time already ended
	}

	return -1;
}

std::string Database::GetAssetDir(std::string a_id) {
	if (verify_auction_id(a_id) == -1) {
		return "";
	}

	std::string asset_dir;

	std::string dir_name = "ASDIR/AUCTIONS/" + a_id;
	dir_name += "/ASSET";

	/* Tratar desta parte eventualmente quando perceber a função*/
	if (fs::is_empty(dir_name)) {
		return "";
	}
	for (const auto &entry : fs::directory_iterator(dir_name)) {
		asset_dir = entry.path();
		break;
	}

	return asset_dir;
}

/* Returns 0 if it belongs, otherwise -1*/
int Database::CheckAuctionBelongs(std::string a_id, std::string user_id) {
	std::string dir_name = "ASDIR/USERS/" + user_id;
	dir_name += "/HOSTED";

	if (fs::is_empty(dir_name)) {
		return -1;
	}
	for (const auto &entry : fs::directory_iterator(dir_name)) {
		std::string aid = entry.path();
		aid.erase(aid.end() - 4, aid.end());
		aid.erase(aid.begin(), aid.end() - 3);

		if (a_id == aid) {
			return 0;
		}
	}

	return -1;
}

/* 0 exists, -1 doesn't*/
int Database::CheckAuctionExists(std::string a_id) {
	std::string a_id_dirname = "ASDIR/AUCTIONS/" + a_id;
	std::cout << "a_id_dirname" << std::endl;

	DIR *dir = opendir((a_id_dirname.c_str()));

	if (dir) {
		closedir(dir);
		return 0;
	} else if (ENOENT == errno) {
		return -1;
	}

	return -1;
}

std::string Database::GetAssetData(std::string a_id, std::string asset_fname) {
	std::ifstream file(asset_fname);
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

int Database::CreateBaseDir() {
	const char *asdir = "ASDIR";
	const char *users = "ASDIR/USERS";
	const char *auctions = "ASDIR/AUCTIONS";

	if (mkdir(asdir, 0700) == -1) {
		return -1;
	}

	if (mkdir(users, 0700) == -1) {
		return -1;
	}

	if (mkdir(auctions, 0700) == -1) {
		return -1;
	}

	return 0;
}

int Database::LoginUser(std::string user_id, std::string password) {
	if (CheckUserLoggedIn(user_id) == 0) {
		if (CorrectPassword(user_id, password) != 1) {
			throw UserNotLoggedIn();
			return DB_LOGIN_NOK;  // Wrong Password
		}
		return DB_LOGIN_OK;
	}
	int created_user = CreateUserDir(user_id);

	if (created_user == -1) {
		return DB_LOGIN_NOK;  // Incorrect login
	}

	if (created_user == 2) {
		if (CheckUserRegistered(user_id) == 0) {
			if (CorrectPassword(user_id, password) != 1) {
				return DB_LOGIN_NOK;  // Wrong Password
			}
			if (CreateLogin(user_id) == -1) {
				return DB_LOGIN_NOK;  // Incorrect login
			}
			return DB_LOGIN_OK;  // Successful login

		} else {
			if (CreatePassword(user_id, password) == -1) {
				return DB_LOGIN_NOK;  // Incorrect login
			}
			if (CreateLogin(user_id) == -1) {
				return DB_LOGIN_NOK;  // Incorrect login
			}
			return DB_LOGIN_REGISTER;  // New user registered
		}
	}

	if (CreatePassword(user_id, password) == -1) {
		return DB_LOGIN_NOK;  // Incorrect login
	}

	if (CreateLogin(user_id) == -1) {
		return DB_LOGIN_NOK;  // Incorrect login
	}

	return DB_LOGIN_REGISTER;  // New user registered
}

int Database::Logout(std::string user_id, std::string password) {
	if (CorrectPassword(user_id, password) != 1) {
		return DB_LOGOUT_NOK;  // Wrong Password
	}

	int removed_login = EraseLogin(user_id);

	if (removed_login == -1) {
		return DB_LOGOUT_UNREGISTERED;  // Unknown user
	}

	if (removed_login == 0) {
		return DB_LOGOUT_OK;  // Successful logout
	}

	if (removed_login == 2) {
		throw UserNotLoggedIn();
		return DB_LOGOUT_NOK;  // User not logged in
	}

	return DB_LOGOUT_NOK;
}

int Database::Unregister(std::string user_id, std::string password) {
	if (CorrectPassword(user_id, password) != 1) {
		return DB_UNREGISTER_NOK;  // Wrong Password
	}

	if (Logout(user_id, password) == DB_LOGOUT_NOK) {
		return DB_UNREGISTER_NOK;  // Couldn't logout when unregistering
	}

	int erased_password = ErasePassword(user_id);

	if (erased_password == -1) {
		return DB_UNREGISTER_NOK;  // Incorrect unregister attempt
	}

	if (erased_password == 0) {
		return DB_UNREGISTER_OK;  // Successful unregister
	}

	if (erased_password == 2) {
		return DB_UNREGISTER_UNKNOWN;  // Unknown user
	}

	return DB_UNREGISTER_NOK;
}

int Database::Open(std::string user_id, std::string name, std::string password,
                   std::string asset_fname, std::string start_value,
                   std::string timeactive, size_t fsize, std::string data) {
	if (CheckUserLoggedIn(user_id) != 0) {
		throw UserNotLoggedIn();
		return DB_OPEN_NOT_LOGGED_IN;  // Not Logged in
	}
	if (CorrectPassword(user_id, password) != 1) {
		return DB_OPEN_CREATE_FAIL;  // Wrong Password
	}
	uint32_t aid = 0;
	std::string new_aid;
	uint32_t n_new_aid;

	std::string dir_name = "ASDIR/AUCTIONS/";

	if (fs::is_empty(dir_name)) {
		aid = 1;
	} else {
		for (const auto &entry : fs::directory_iterator(dir_name)) {
			new_aid = entry.path();
			new_aid.erase(new_aid.begin(), new_aid.end() - 3);
			n_new_aid = stoi(new_aid);

			if (aid <= n_new_aid) {
				aid = n_new_aid + 1;
			}
		}
	}

	std::string c_aid = convert_auction_id_to_str(aid);

	std::cout << "A" << std::endl;
	if (CreateAuctionDir(c_aid) == -1) {
		return DB_OPEN_CREATE_FAIL;  // Failed to create auction dir
	}
	std::cout << "B" << std::endl;
	if (CreateStartFile(c_aid, user_id, name, asset_fname, start_value,
	                    timeactive) == -1) {
		return DB_OPEN_CREATE_FAIL;  // Failed to create start file
	}
	std::cout << "C" << std::endl;
	if (CreateAssetFile(c_aid, asset_fname, fsize, data) == -1) {
		return DB_OPEN_CREATE_FAIL;  // Failed to create asset file
	}
	std::cout << "D" << std::endl;

	if (RegisterHost(user_id, c_aid) == -1) {
		return DB_OPEN_CREATE_FAIL;  // Failed to create hosted file
	}

	return aid;
}

int Database::CloseAuction(std::string a_id, std::string user_id,
                           std::string password) {
	std::string user_id_dir = "ASDIR/USERS/" + user_id;
	const char *user_id_dirname = user_id_dir.c_str();
	if (CheckUserExisted(user_id_dirname) == -1) {
		std::cout << "dis shit" << std::endl;
		throw UserDoesNotExist();
		return DB_CLOSE_NOK;  // User doesn't exist
	}
	if (CheckUserLoggedIn(user_id) != 0) {
		std::cout << "dis shit2" << std::endl;
		throw UserNotLoggedIn();
		return DB_CLOSE_NOK;  // Not Logged in
	}
	if (CorrectPassword(user_id, password) != 1) {
		std::cout << "dis shit3" << std::endl;
		throw IncorrectPassword();
		return DB_CLOSE_NOK;  // Wrong Password
	}
	if (CheckAuctionExists(a_id) == -1) {
		std::cout << "dis shit4" << std::endl;
		throw AuctionNotFound();
		return DB_CLOSE_NOK;
	}
	if (CheckAuctionBelongs(a_id, user_id) == -1) {
		std::cout << "dis shit5" << std::endl;
		throw AuctionNotOwnedByUser();
		return DB_CLOSE_NOK;
	}

	std::string dir_name = "ASDIR/AUCTIONS/" + a_id;
	dir_name += "/END";
	dir_name += a_id;
	dir_name += ".txt";
	if (CheckEndExists(dir_name.c_str()) == 0) {
		std::cout << "dis shit6" << std::endl;
		throw AuctionAlreadyClosed();
		return DB_CLOSE_NOK;
	}

	return (Close(a_id));
}

AuctionList Database::MyAuctions(std::string user_id) {
	std::string dir_name = "ASDIR/USERS/" + user_id;
	dir_name += "/HOSTED";

	AuctionList result;
	AuctionListing auction;
	StartInfo start;
	time_t fulltime;

	if (fs::is_empty(dir_name)) {
		return result;  // Returns empty list which means user hosted no
		                // auctions
	} else {
		for (const auto &entry : fs::directory_iterator(dir_name)) {
			std::string aid = entry.path();
			aid.erase(aid.end() - 4, aid.end());
			aid.erase(aid.begin(), aid.end() - 3);

			auction.a_id = aid;
			std::string dir_name = "ASDIR/AUCTIONS/" + aid;
			dir_name += "/END_";
			dir_name += aid;
			dir_name += ".txt";

			if (CheckEndExists(dir_name.c_str()) == -1) {
				if (GetStart(aid, start) == -1) {
					throw AuctionNotFound();
					return result;
				};
				uint32_t start_time = start.current_time;
				time_t current_time = time(&fulltime);
				uint32_t time_passed = current_time - start_time;
				uint32_t timeactive = stoi(start.timeactive);

				if (time_passed >= timeactive) {
					Close(aid);
					auction.active = false;
				} else {
					auction.active = true;
				}
			} else {
				auction.active = false;
			}

			result.push_back(auction);
		}
	}
	return result;
}

AuctionList Database::MyBids(std::string user_id) {
	std::string dir_name = "ASDIR/USERS/" + user_id;
	dir_name += "/BIDDED";

	AuctionList result;
	AuctionListing auction;
	StartInfo start;
	time_t fulltime;

	if (fs::is_empty(dir_name)) {
		return result;  // Returns empty list which means user made no bids
	} else {
		for (const auto &entry : fs::directory_iterator(dir_name)) {
			std::string aid = entry.path();
			aid.erase(aid.end() - 4, aid.end());
			aid.erase(aid.begin(), aid.end() - 3);

			auction.a_id = aid;
			std::string dir_name = "ASDIR/AUCTIONS/" + aid;
			dir_name += "/END_";
			dir_name += aid;
			dir_name += ".txt";

			if (CheckEndExists(dir_name.c_str()) == -1) {
				if (GetStart(aid, start) == -1) {
					throw AuctionNotFound();
					return result;
				};
				uint32_t start_time = start.current_time;
				time_t current_time = time(&fulltime);
				uint32_t time_passed = current_time - start_time;
				uint32_t timeactive = stoi(start.timeactive);

				if (time_passed >= timeactive) {
					Close(aid);
					auction.active = false;
				} else {
					auction.active = true;
				}
			} else {
				auction.active = false;
			}

			result.push_back(auction);
		}
	}
	return result;
}

AuctionList Database::List() {
	std::string dir_name = "ASDIR/AUCTIONS";

	AuctionList result;
	AuctionListing auction;
	StartInfo start;
	time_t fulltime;

	if (fs::is_empty(dir_name)) {
		return result;  // Returns empty list which means no auctions were ever
		                // made
	} else {
		for (const auto &entry : fs::directory_iterator(dir_name)) {
			std::string aid = entry.path();
			aid.erase(aid.begin(), aid.end() - 3);

			auction.a_id = aid;
			std::string dir_name = "ASDIR/AUCTIONS/" + aid;
			dir_name += "/END_";
			dir_name += aid;
			dir_name += ".txt";

			if (CheckEndExists(dir_name.c_str()) == -1) {
				if (GetStart(aid, start) == -1) {
					throw AuctionNotFound();
					return result;
				};
				uint32_t start_time = start.current_time;
				time_t current_time = time(&fulltime);
				uint32_t time_passed = current_time - start_time;
				uint32_t timeactive = stoi(start.timeactive);

				if (time_passed >= timeactive) {
					Close(aid);
					auction.active = false;
				} else {
					auction.active = true;
				}
			} else {
				auction.active = false;
			}

			result.push_back(auction);
		}
	}
	return result;
}

AssetInfo Database::ShowAsset(std::string a_id) {
	AssetInfo asset;

	std::string asset_dir = GetAssetDir(a_id);

	if (asset_dir == "") {
		throw AssetDoesNotExist();
		return DB_SHOW_ASSET_ERROR;
	}

	asset_dir.erase(asset_dir.begin(), asset_dir.begin() + 25);

	asset.fdata = GetAssetData(a_id, asset_dir);
	asset.fsize = (asset.fdata).size();

	return asset;
}

int Database::Bid(std::string user_id, std::string password, std::string a_id,
                  std::string value) {
	if (CheckUserLoggedIn(user_id) != 0) {
		throw UserNotLoggedIn();
		return DB_BID_NOK;  // Not Logged in
	}
	if (CorrectPassword(user_id, password) != 1) {
		return DB_BID_NOK;  // Wrong Password
	}
	if (CheckAuctionBelongs(a_id, user_id) == 0) {
		throw AuctionNotOwnedByUser();
		return DB_BID_NOK;
	}

	BidInfo bid;

	std::string end_dir_name = "ASDIR/AUCTIONS/" + a_id;
	end_dir_name += "/END_";
	end_dir_name += a_id;
	end_dir_name += ".txt";

	if (CheckEndExists(end_dir_name.c_str()) == 0) {
		throw AuctionAlreadyClosed();
		return DB_BID_NOK;
	}

	std::string bid_dir_name = "ASDIR/AUCTIONS/" + a_id;
	bid_dir_name += "/BIDS";

	std::string bid_fname;

	if (!(fs::is_empty(bid_dir_name))) {
		for (const auto &entry : fs::directory_iterator(bid_dir_name)) {
			bid_fname += entry.path();
			GetBid(bid_fname, bid);
			std::string old_value = bid.value;

			if (old_value >= value) {
				throw LargerBidAlreadyExists();
				return DB_BID_NOK;
			}
		}
	}

	if (RegisterBid(user_id, a_id) == -1) {
		return DB_BID_REFUSE;
	}

	if (CreateBidFile(a_id, user_id, value) == -1) {
		return DB_BID_REFUSE;
	}

	return DB_BID_ACCEPT;
}

AuctionRecord Database::ShowRecord(std::string a_id) {
	BidInfo bid;
	time_t fulltime;
	StartInfo start;
	EndInfo end;
	uint32_t finished;
	uint32_t n = 0;
	AuctionRecord result;
	BidList list;

	if (GetStart(a_id, start) == -1) {
		throw AuctionNotFound();
		return result;
	};

	result.auction_name = start.name;
	result.host_id = start.user_id;
	result.asset_fname = start.asset_fname;
	result.start_value = start.start_value;
	result.start_datetime = start.current_date;
	result.timeactive = start.timeactive;
	uint32_t start_time = start.current_time;
	time_t current_time = time(&fulltime);
	uint32_t time_passed = current_time - start_time;
	uint32_t timeactive = stoi(start.timeactive);

	std::string end_dir_name = "ASDIR/AUCTIONS/" + a_id;
	end_dir_name += "/END_";
	end_dir_name += a_id;
	end_dir_name += ".txt";

	if (time_passed >= timeactive) {
		if (CheckEndExists(end_dir_name.c_str()) == -1) {
			Close(a_id);
			result.active = false;
			result.end_datetime = current_time;
			finished = time_passed - timeactive;
			result.end_timeelapsed = finished;
		} else {
			GetEnd(end_dir_name.c_str(), end);
			result.active = false;
			result.end_datetime = end.end_date;
			result.end_timeelapsed = end.end_time;
		}
	} else {
		result.active = true;
	}

	std::string dir_name = "ASDIR/AUCTIONS/" + a_id;
	dir_name += "/BIDS";

	std::string bid_fname;
	for (const auto &entry : fs::directory_iterator(dir_name)) {
		n++;
		bid_fname += entry.path();
		GetBid(bid_fname, bid);

		list.push_back(bid);

		if (n = 50) {
			break;
		}
	}

	result.list = list;
	return result;
}
