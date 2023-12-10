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

#include "shared/verifications.hpp"

namespace fs = std::filesystem;

int Database::CheckUserExisted(const char *user_id_dirname) {
	DIR *dir = opendir(user_id_dirname);

	if (dir) {
		closedir(dir);
		return 0;
	} else if (ENOENT == errno) {
		return -1;
	}

	return -1;
}

/* Returns -1 if failed, 0 if sucessful, 2 if user already existed*/
int Database::CreateUserDir(std::string user_id) {
	if (verify_user_id(user_id) == -1) {
		std::cerr << "Invalid user id " << user_id << std::endl;
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
		std::cerr << "mkdir1" << std::endl;
		return -1;
	}

	if (mkdir(hosted_dirname, 0700) == -1) {
		std::cerr << "mkdir2" << std::endl;
		return -1;
	}

	if (mkdir(bidded_dirname, 0700) == -1) {
		std::cerr << "mkdir3" << std::endl;
		return -1;
	}

	return 0;
}

int Database::CreateAuctionDir(std::string a_id) {
	if (verify_auction_id(a_id) == -1) {
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
		return -1;
	}
	if (mkdir(bid_dirname, 0700) == -1) {
		return -1;
	}
	if (mkdir(asset_dirname, 0700) == -1) {
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
	user_id_name += "/_login.txt";

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

	FILE *fp;

	std::string password_name = "ASDIR/USERS/" + user_id;
	password_name += "/";
	password_name += user_id;
	password_name += "/_pass.txt";

	const char *password_fname = password_name.c_str();
	const char *password_file = password.c_str();

	fp = fopen(password_fname, "w");
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
	host_name += "/HOSTED";
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
	bid_name += "/BIDDED";
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
	login_id_name += "/_login.txt";

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
	password_name += "/_pass.txt";

	const char *password_fname = password_name.c_str();

	if (CheckPasswordExists(password_fname) == -1) {
		return -1;
	}

	unlink(password_fname);

	return 0;
}

int Database::CheckAssetFile(std::string asset_fname) {
	if (verify_asset_fname(asset_fname) == -1) {
		return -1;
	}

	struct stat filestat;

	const char *asset_file_name = asset_fname.c_str();

	if (stat(asset_file_name, &filestat) == -1 || filestat.st_size == 0) {
		return -1;
	}

	return (filestat.st_size);
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

	if (CheckAssetFile(asset_fname) == -1) {
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
	uint32_t current_time = time(&fulltime);

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
	content += current_time;

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
	Start start;
	time_t fulltime;
	std::string current_date = GetCurrentDate();
	uint32_t current_time = time(&fulltime);
	GetStart(a_id, start);
	uint32_t start_time = start.current_time;
	uint32_t time_passed = start_time - current_time;

	std::string dir_name = "ASDIR/AUCTIONS/" + a_id;
	dir_name += "/END_";
	dir_name += a_id;
	dir_name += ".txt";

	std::string content = current_date + " ";
	content += time_passed;

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

int Database::CreateAssetFile(std::string a_id, std::string asset_fname) {
	if (CheckAssetFile(asset_fname) == -1) {
		return -1;
	}

	if (verify_auction_id(a_id) == -1) {
		return -1;
	}

	FILE *fp;

	std::string asset_file = "ASDIR/AUCTIONS/" + a_id;
	asset_file += "/ASSET/";
	asset_file += asset_fname;

	const char *asset_file_name = asset_file.c_str();

	fp = fopen(asset_file_name, "w");
	if (fp == NULL) {
		return -1;
	}

	// Copy over the file from the fname sent

	fclose(fp);

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
	Start start;
	FILE *fp;
	time_t fulltime;
	std::string current_date = GetCurrentDate();
	uint32_t current_time = time(&fulltime);
	GetStart(a_id, start);
	uint32_t start_time = start.current_time;
	uint32_t time_passed = start_time - current_time;

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

int Database::GetStart(std::string a_id, Start &result) {
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

int Database::GetBid(std::string bid_fname, Bid &result) {
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
	char time_str[20];
	char *current_date;
	time(&fulltime);  // Get current time in seconds starting at 1970
	current_time = gmtime(&fulltime);  // Convert time to YYYY−MM−DD HH:MM:SS
	sprintf(current_date, "%4d-%02d-%02d %02d:%02d:%02d",
	        current_time->tm_year + 1900, current_time->tm_year + 1,
	        current_time->tm_mday, current_time->tm_hour, current_time->tm_min,
	        current_time->tm_sec);
	std::string str(current_date);
	return current_date;
}

// 1 is logged in, 0 is not logged in, -1 is error
int Database::UserLoggedIn(std::string user_id) {
	if (verify_user_id(user_id) == -1) {
		return -1;
	}

	std::string login_name = "ASDIR/USERS/" + user_id;
	login_name += "/";
	login_name += user_id;
	login_name += "/_login.txt";

	const char *login_fname = login_name.c_str();

	DIR *dir = opendir(login_fname);

	if (dir) {
		closedir(dir);
		return 1;
	} else if (ENOENT == errno) {
		return 0;
	}

	return -1;
}

// 1 it is, 0 is not, -1 is error
int Database::UserRegistered(std::string user_id) {
	if (verify_user_id(user_id) == -1) {
		return -1;
	}

	std::string pass_name = "ASDIR/USERS/" + user_id;
	pass_name += "/";
	pass_name += user_id;
	pass_name += "/_pass.txt";

	const char *pass_fname = pass_name.c_str();

	DIR *dir = opendir(pass_fname);

	if (dir) {
		closedir(dir);
		return 1;
	} else if (ENOENT == errno) {
		return 0;
	}

	return -1;
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
	pass_name += "/_pass.txt";

	const char *pass_fname = pass_name.c_str();

	fp = fopen(pass_fname, "w");
	if (fp == NULL) {
		return -1;
	}

	if (fgets(content, 8, fp) == NULL) {
		return -1;
	}

	fclose(fp);

	if (content == password) {
		return 1;
	} else {
		return 0;
	}
}

std::string Database::GetAssetFname(std::string a_id) {
	if (verify_auction_id(a_id) == -1) {
		return "";
	}

	std::string asset_fname;

	std::string dir_name = "ASDIR/AUCTIONS/" + a_id;
	dir_name += "/ASSET";

	/* Tratar desta parte eventualmente quando perceber a função*/
	for (const auto &entry : fs::directory_iterator(dir_name)) {
		asset_fname = entry.path();
		break;
	}

	if (CheckAssetFile(asset_fname) == -1) {
		return "";
	}

	return asset_fname;
}

std::stringstream Database::GetAssetData(std::string a_id,
                                         std::string asset_fname) {
	std::string dir_name = "ASDIR/AUCTIONS/" + a_id;
	dir_name += "/ASSET/";
	dir_name += asset_fname;

	const char *dir_fname = dir_name.c_str();

	std::ifstream asset_file(dir_fname);

	std::stringstream asset_data;

	asset_data << asset_file.rdbuf();

	asset_file.close();

	return asset_data;
}

/*  Juntar num create auction que cria tudo da auction
    Juntar num create user que cria tudo do user
    Juntar num close auction geral
    Juntar num close user geral
    List auctions (use cpp thing): user's auctions; all auctions; aunctions in
    which user bidded
    Fazer wrappers (aka passes parameters and looks neat for interace)
    Ver outros commandos
    */

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
	int created_user = CreateUserDir(user_id);

	if (created_user == -1) {
		std::cerr << "Error creating user directory" << std::endl;
		return -1;  // Incorrect login
	}

	if (CreatePassword(user_id, password) == -1) {
		std::cerr << "Error creating password file" << std::endl;
		return -1;  // Incorrect login
	}

	if (CreateLogin(user_id) == -1) {
		std::cerr << "Error creating login file" << std::endl;
		return -1;  // Incorrect login
	}

	if (created_user == 2) {
		std::cerr << "User already existed" << std::endl;
		return 2;  // Successful login
	}

	return 0;  // New user registered
}

int Database::Logout(std::string user_id) {
	int removed_login = EraseLogin(user_id);

	if (removed_login == -1) {
		return -1;  // Unknown user
	}

	if (removed_login == 0) {
		return 0;  // Successful logout
	}

	if (removed_login == 2) {
		return 2;  // User not logged in
	}

	return -1;
}

int Database::Unregister(std::string user_id) {
	int erased_password = ErasePassword(user_id);

	if (erased_password == -1) {
		return -1;  // Incorrect unregister attempt
	}

	if (erased_password == 0) {
		return 0;  // Successful unregister
	}

	if (erased_password == 2) {
		return 2;  // Unknown user
	}

	return -1;
}

int Database::Open(std::string user_id, std::string name,
                   std::string asset_fname, std::string start_value,
                   std::string timeactive) {
	// need to determine a way to make an AID and finish other opening stuff

	return 0;
}

int Database::Close(std::string a_id) {
	int ended = CreateEndFile(a_id);

	if (ended == -1) {
		return -1;  // Error
	}

	if (ended == 0) {
		return 0;  // Auction successfully closed
	}

	if (ended == 2) {
		return 2;  // Auction time already ended
	}

	return -1;
}

std::string Database::ShowRecord(std::string a_id) {
	if (verify_auction_id(a_id) == -1) {
		return "";
	}

	time_t fulltime;
	Start start;
	uint32_t finished;
	uint32_t n = 0;

	GetStart(a_id, start);
	std::string name = start.name;
	std::string value = start.start_value;
	uint32_t start_time = start.current_time;
	uint32_t current_time = time(&fulltime);
	uint32_t time_passed = start_time - current_time;
	uint32_t timeactive = stoi(start.timeactive);
	uint32_t overtime = time_passed - timeactive;

	std::string content = name + " ";
	content += value;
	content += " ";
	content += std::to_string(start_time);
	content += " ";
	content += timeactive;
	content += "\n";

	if (overtime >= 0) {
		Close(a_id);
		finished = 1;
	} else {
		finished = 0;
	}

	std::string dir_name = "ASDIR/AUCTIONS/" + a_id;
	dir_name += "/BIDS";

	std::string bid_fname;

	for (const auto &entry : fs::directory_iterator(dir_name)) {
		n++;
		bid_fname = entry.path();
		Bid bid;
		GetBid(bid_fname, bid);

		content += bid.user_id;
		content += " ";
		content += bid.value;
		content += " ";
		content += bid.current_date;
		content += "\n";

		if (n = 50) {
			break;
		}
	}

	if (finished == 1) {
		content += "The auction was finished ";
		content += overtime;
		content += " seconds ago.";
	}

	return content;
}