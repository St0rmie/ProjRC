#include "database.hpp"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
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

#include "shared/utils.hpp"
#include "shared/verifications.hpp"

namespace fs = std::filesystem;

/**
 * @brief  Initializes the semaphore.
 * @param  lock_id: The id of the lock.
 * @retval -1 if it fails.
 * @retval 0 if it succeeds.
 */
int Database::semaphore_init(int lock_id) {
	std::string sem_name = "sem_AS_" + std::to_string(lock_id);
	sem_unlink(sem_name.c_str());

	_sem = sem_open(sem_name.c_str(), O_CREAT, 0644, 1);

	if (_sem == SEM_FAILED) {
		return -1;
	}
	return 0;
}

/**
 * @brief  Locks the semaphore.
 * @throws If it can't lock it throws a SemException.
 * @retval None
 */
void Database::semaphore_wait() {
	if (sem_wait(_sem) == -1)
		throw SemException();
}

/**
 * @brief  Unlocks the semaphore
 * @throws If it can't unlock then it throws a SemException.
 * @retval None
 */
void Database::semaphore_post() {
	if (sem_post(_sem) == -1)
		throw SemException();
}

/**
 * @brief  Destroys the semaphore.
 * @retval 0 when successful.
 */
int Database::semaphore_destroy() {
	std::string sem_name = "sem_AS_" + std::to_string(_lock_id);
	sem_close(_sem);
	return 0;
}

/**
 * @brief  Compares the auctions by their auction ids in order to sort them.
 * @param  &a: First auction.
 * @param  &b: Second auction.
 * @retval true if auction id of the first auction is smaller than the.
   second's.
 * @retval false if otherwise.
 */
bool CompareByAid(const AuctionListing &a, const AuctionListing &b) {
	uint32_t a_aid = static_cast<uint32_t>(stoi(a.a_id));
	uint32_t b_aid = static_cast<uint32_t>(stoi(b.a_id));
	return (a_aid < b_aid);
}

/**
 * @brief  Compares the bids by their auction ids in order to sort them.
 * @param  &a: First bid.
 * @param  &b: Second bid.
 * @retval true if auction id of the first bid is smaller than the.
   second's.
 * @retval false if otherwise.
 */
bool CompareByValue(const BidInfo &a, const BidInfo &b) {
	uint32_t a_aid = static_cast<uint32_t>(stol(a.value));
	uint32_t b_aid = static_cast<uint32_t>(stol(b.value));
	return (a_aid < b_aid);
}

/**
 * @brief Checks if the user exists or existed at one point (if they
 unregistered or logged out).
 * @param  *user_id_dirname: The path of the directory of the supposed user.
 * @retval -1 if the user doesn't or didn't exist.
 * @retval 0 if the user exists or existed at one point.
 */
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

/**
 * @brief Checks if the user is registered.
 * @param  user_id: The path of the directory of the supposed user.
 * @retval -1 if the id is invalid or the user isn't registered.
 * @retval 0 if the user is registered.
 */
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

/**
 * @brief  Checks if the user is logged in.
 * @param  user_id: The user's id.
 * @retval	-1 if the id is invalid doesn't exist or the user isn't logged in.
 * @retval	0 if the user is logged in.
 */
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

/**
 * @brief  Creates the directory of the user.
 * @param  user_id: The user's id.
 * @retval -1 if the id is invalid or the directory isn't properly created.
 * @retval 0 if the creation is successful.
 * @retval 2 if the directory already existed.
 */

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

/**
 * @brief  Creates the directory of the auction.
 * @param  a_id: The auction's id.
 * @retval -1 if the id is invalid or the directory isn't properly created.
 * @retval 0 if the creation is successful.
 */
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

/**
 * @brief  Creates the login file.
 * @param  user_id: The user's id.
 * @retval -1 if the id is invalid or the file isn't properly created.
 * @retval 0 if the creation is successful.
 */
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

/**
 * @brief  Creates the password file.
 * @param  user_id: The user's id.
 * @param  password: The user's password.
 * @retval -1 if the password is invalid or the file isn't properly created.
 * @retval 0 if the creation is successful
 */
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

/**
 * @brief  Creates a file the corresponds to the auction the user hosts.
 * @param  user_id: The user's id.
 * @param  a_id: The auction's id.
 * @retval -1 if either id is invalid or the file isn't properly created.
 * @retval 0 if the creation is successful.
 */
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

/**
 * @brief  Creates a file that corresponds to the auction the user bid on.
 * @param  user_id: The user's id.
 * @param  a_id: The auction's id.
 * @retval -1 if either id is invalid or the file isn't properly created.
 * @retval 0 if the creation is successful.
 */
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

/**
 * @brief  Checks if the login file exists.
 * @param  *login_id_fname: The path to the login file.
 * @retval -1 if the file doesn't exist.
 * @retval 0 if it exists.
 */
int Database::CheckLoginExists(const char *login_id_fname) {
	if (access(login_id_fname, F_OK) == 0) {
		return 0;
	} else {
		return -1;
	}
}

/**
 * @brief  Removes the login file from the user.
 * @param  user_id: The user's id.
 * @retval -1 if the id is invalid or the user doesn't exist.
 * @retval 0 if the removal is successful.
 * @retval 2 if the login file has already been removed.
 */
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

/**
 * @brief  Checks if the password file exists.
 * @param  *password_fname: The path to the password file.
 * @retval -1 if the file doesn't exist.
 * @retval 0 if it exists.
 */
int Database::CheckPasswordExists(const char *password_fname) {
	if (access(password_fname, F_OK) == 0) {
		return 0;
	} else {
		return -1;
	}
}

/**
 * @brief  Removes the password file from the user.
 * @param  user_id: The user's id.
 * @retval -1 if the id is invalid or the password file has already been
 * removed.
 * @retval 0 if the removal is successful.
 * @retval 2 if the user doesn't exist.
 */
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

/**
 * @brief  Creates the auction's start file.
 * @param  a_id: The auction's id.
 * @param  user_id: The user's id.
 * @param  name: The name of the asset auctioned.
 * @param  asset_fname: The path to the asset's image file.
 * @param  start_value: The starting value for bids.
 * @param  timeactive: The time the auction will be active for.
 * @retval -1 if any parameters are invalid or the file isn't properly created.
 * @retval 0 if the creation is successful.
 */
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

/**
 * @brief  Checks if the end file exists.
 * @param  *end_fname: The path to the end file.
 * @retval -1 if the file doesn't exist.
 * @retval 0 if it exists.
 */
int Database::CheckEndExists(const char *end_fname) {
	if (access(end_fname, F_OK) == 0) {
		return 0;
	} else {
		return -1;
	}
}

/**
 * @brief Creates the auction's end file.
 * @param  a_id: The auction's id.
 * @throws AuctionNotFound if the auction doesn't exist.
 * @retval -1 if the auction's id is invalid, if the auction doesn't exist or
 * the file isn't properly created.
 * @retval 0 if the creation is successful.
 * @retval 2 if the end file already exists.
 */
int Database::CreateEndFile(std::string a_id) {
	if (verify_auction_id(a_id) == -1) {
		return -1;
	}

	FILE *fp;
	StartInfo start;
	time_t fulltime;
	struct tm *finish_time;
	std::string current_date = GetCurrentDate();
	std::string content;
	char date_str[40];
	uint32_t current_time = static_cast<uint32_t>(time(&fulltime));
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

	uint32_t supposed_end = static_cast<uint32_t>(stol(start.timeactive));

	if (time_passed > supposed_end) {
		// If more time has passed than the suposed duration of an auction,
		// when creating the end file the date of the supposed end will be
		// calculated.

		time_t total_time = static_cast<time_t>(start_time + supposed_end);
		finish_time = gmtime(&total_time);
		sprintf(date_str, "%4u-%02u-%02u %02u:%02u:%02u",
		        finish_time->tm_year + 1900, finish_time->tm_mon + 1,
		        finish_time->tm_mday, finish_time->tm_hour, finish_time->tm_min,
		        finish_time->tm_sec);

		content = date_str;
		content += " ";
		content += std::to_string(supposed_end);
	} else {
		content = current_date + " ";
		content += std::to_string(time_passed);
	}

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

/**
 * @brief  Creates a copy of the asset.
 * @param  a_id: The auction's id.
 * @param  asset_fname: The path to the asset's image file.
 * @param  data: The asset's image data.
 * @retval -1 if the auction's id is invalid or the file isn't created properly.
 * @retval 0 if the creation is successful.
 */
int Database::CreateAssetFile(std::string a_id, std::string asset_fname,
                              std::string data) {
	if (verify_auction_id(a_id) == -1) {
		return -1;
	}

	std::string dir_name = "ASDIR/AUCTIONS/" + a_id;
	dir_name += "/ASSET/";
	dir_name += asset_fname;

	std::ofstream file;

	file.open(dir_name, std::ofstream::trunc);
	if (!file.good()) {
		return -1;
	}

	file << data;
	file.close();

	return 0;
}

/**
 * @brief  Creates the bid file.
 * @param  a_id: The auction's id.
 * @param  user_id: The user's id.
 * @param  value: The value of the bid.
 * @throws AuctionNotFound if the auction doesn't exist.
 * @retval -1 if either id is invalid, the value is invalid, the auction doesn't
 * exist, or the file isn't properly created.
 * @retval 0 if the creation is successful.
 */
int Database::CreateBidFile(std::string a_id, std::string user_id,
                            std::string value) {
	if (verify_value(static_cast<uint32_t>(stol(value))) == -1) {
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
	uint32_t current_time = static_cast<uint32_t>(time(&fulltime));
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

/**
 * @brief  Gets the information of the start file.
 * @param  a_id: The auction's id.
 * @param  &result: The struct in which the info will be stored.
 * @retval -1 if the file doesn't exist, is empty or isn't properly formated.
 * @retval 0 if the retrieval is successful.
 */
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
	result.current_time = static_cast<uint32_t>(stol(parsed_content[7]));

	fclose(fp);

	return 0;
}

/**
 * @brief  Gets the information of the end file.
 * @param  *end_fname: The path to the end file.
 * @param  &end: The struct in which the info will be stored.
 * @retval -1 if the file doesn't exist, is empty or has invalid format.
 * @retval 0 if the retrieval is successful.
 */
int Database::GetEnd(const char *end_fname, EndInfo &end) {
	FILE *fp;
	char content[200];
	fp = fopen(end_fname, "r");
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

	if (parsed_content.size() != 3) {
		return -1;
	}

	end.end_date = parsed_content[0] + " ";
	end.end_date += parsed_content[1];
	end.end_time = static_cast<uint32_t>(stol(parsed_content[2]));

	fclose(fp);
	return 0;
}

/**
 * @brief  Gets the information of the bid file.
 * @param  bid_fname: The path to the bid file.
 * @param  &result: The struct in which the info will be stored.
 * @retval -1 if the file doesn't exist, is empty or has invalid format.
 * @retval 0 if the retrieval is successful.
 */
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
	result.time_passed = static_cast<uint32_t>(stol(parsed_content[4]));

	fclose(fp);

	return 0;
}

/**
 * @brief  Gets the current date and time.
 * @retval The date obtained.
 */
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
	return str;
}

/**
 * @brief  Checks whether the password given is the user's password.
 * @param  user_id: The user's id.
 * @param  password: The user's password.
 * @retval -1 if the user or the passsword are invalid, if the password file
 * * doesn't open or is empty.
 * @retval 0 if the password is incorrect.
 * @retval 1 if the password is correct.
 */
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

/**
 * @brief  Closes the auction.
 * @param  a_id: The auction's id.
 * @throws AuctionAlreadyClosed if the auction already ended.
 * @retval DB_CLOSE_NOK if there's an error in closing.
 * @retval DB_CLOSE_OK if the auction closes successfully.
 * @retval DB_CLOSE_ENDED_ALREADY if the auction was already closed.
 */
int Database::Close(std::string a_id) {
	int ended = CreateEndFile(a_id);

	if (ended == -1) {
		return DB_CLOSE_NOK;
	}

	if (ended == 0) {
		return DB_CLOSE_OK;
	}

	if (ended == 2) {
		throw AuctionAlreadyClosed();
		return DB_CLOSE_ENDED_ALREADY;
	}

	return -1;
}

/**
 * @brief  Gets the path to the image of the asset.
 * @param  a_id: The auction's id.
 * @retval The path to the image of the auction's asset or an empty string if
 * the auction is invalid or auction has no asset.
 */
std::string Database::GetAssetDir(std::string a_id) {
	if (verify_auction_id(a_id) == -1) {
		return "";
	}

	std::string asset_dir = "";
	std::string dir_name = "ASDIR/AUCTIONS/" + a_id;

	bool auction_found = false;
	for (const auto &entry : fs::directory_iterator("ASDIR/AUCTIONS")) {
		if (dir_name == entry.path())
			auction_found = true;
	}
	if (auction_found == false)
		return "";

	dir_name += "/ASSET";

	if (fs::is_empty(dir_name)) {
		return "";
	}
	for (const auto &entry : fs::directory_iterator(dir_name)) {
		asset_dir = entry.path();
		break;
	}

	return asset_dir;
}

/**
 * @brief  Checks whether the auction belongs to the user.
 * @param  a_id: The auction's id.
 * @param  user_id: The user's id.
 * @retval -1 if the auction does not belong to the user.
 * @retval 0 if the auction belongs to the user.
 */
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

/**
 * @brief  Checks whether the auction exists.
 * @param  a_id: The auction's id.
 * @retval -1 if the auction doesn't exist.
 * @retval 0 if the auction exists.
 */
int Database::CheckAuctionExists(std::string a_id) {
	std::string a_id_dirname = "ASDIR/AUCTIONS/" + a_id;

	DIR *dir = opendir((a_id_dirname.c_str()));

	if (dir) {
		closedir(dir);
		return 0;
	} else if (ENOENT == errno) {
		return -1;
	}

	return -1;
}

/**
 * @brief  Gets the data of the asset's image.
 * @param  asset_fname: The path to the asset.
 * @retval The asset's image data.
 */
std::string Database::GetAssetData(std::string asset_fname) {
	std::ifstream file(asset_fname);
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

/**
 * @brief  Creates the necessary directories for the system to function and
 * initializes the semaphore.
 * @param  sem_id: The semaphore's id.
 * @retval -1 if the semaphore isn't initialized or the directories' creation
 * fails.
 */
int Database::CreateBaseDir(int sem_id) {
	const char *asdir = "ASDIR";
	const char *users = "ASDIR/USERS";
	const char *auctions = "ASDIR/AUCTIONS";

	if (semaphore_init(sem_id) == -1) {
		return -1;
	}

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

/**
 * @brief  Logs the user into the system, creating a new account if the user
 * isn't yet registered.
 * @param  user_id: The user's id.
 * @param  password: The user's password.
 * @throws UserNotLoggedIn if the user isn't logged in.
 * @retval DB_LOGIN_NOK if the user isn't logged in or the password is wrong.
 * @retval DB_LOGIN_OK if the login is successful.
 * @retval DB_LOGIN_REGISTER if a new user is registered.
 */
int Database::LoginUser(std::string user_id, std::string password) {
	semaphore_wait();
	if (CheckUserLoggedIn(user_id) == 0) {
		if (CorrectPassword(user_id, password) != 1) {
			semaphore_post();
			throw UserNotLoggedIn();
			return DB_LOGIN_NOK;
		}
		semaphore_post();
		return DB_LOGIN_OK;
	}
	int created_user = CreateUserDir(user_id);

	if (created_user == -1) {
		semaphore_post();
		return DB_LOGIN_NOK;
	}

	if (created_user == 2) {
		if (CheckUserRegistered(user_id) == 0) {
			if (CorrectPassword(user_id, password) != 1) {
				semaphore_post();
				return DB_LOGIN_NOK;
			}
			if (CreateLogin(user_id) == -1) {
				semaphore_post();
				return DB_LOGIN_NOK;
			}
			semaphore_post();
			return DB_LOGIN_OK;

		} else {
			if (CreatePassword(user_id, password) == -1) {
				semaphore_post();
				return DB_LOGIN_NOK;
			}
			if (CreateLogin(user_id) == -1) {
				semaphore_post();
				return DB_LOGIN_NOK;
			}
			semaphore_post();
			return DB_LOGIN_REGISTER;
		}
	}

	if (CreatePassword(user_id, password) == -1) {
		semaphore_post();
		return DB_LOGIN_NOK;
	}

	if (CreateLogin(user_id) == -1) {
		semaphore_post();
		return DB_LOGIN_NOK;
	}

	semaphore_post();
	return DB_LOGIN_REGISTER;
}

/**
 * @brief  Logs out the user.
 * @param  user_id: The user's id.
 * @param  password: The user's password.
 * @throws UserNotLoggedIn if the user isn't logged in.
 * @retval DB_LOGOUT_NOK if the password is wrong or the user is already logged
 * out.
 * @retval DB_LOGOUT_UNREGISTERED if the user isn't registered.
 * @retval DB_LOGOUT_OK if the logout is successful.
 */
int Database::Logout(std::string user_id, std::string password) {
	semaphore_wait();
	if (CorrectPassword(user_id, password) != 1) {
		semaphore_post();
		return DB_LOGOUT_NOK;
	}

	int removed_login = EraseLogin(user_id);

	if (removed_login == -1) {
		semaphore_post();
		return DB_LOGOUT_UNREGISTERED;
	}

	if (removed_login == 0) {
		semaphore_post();
		return DB_LOGOUT_OK;
	}

	if (removed_login == 2) {
		semaphore_post();
		throw UserNotLoggedIn();
		return DB_LOGOUT_NOK;
	}

	semaphore_post();
	return DB_LOGOUT_NOK;
}

/**
 * @brief  Unregisters the user.
 * @param  user_id: The user's id.
 * @param  password: The user's password.
 * @retval DB_UNREGISTER_NOK if the password is wrong, the logout fails or the
 * user is already unregistered.
 * @retval DB_UNREGISTER_OK if the user is sucessfully unregistered.
 * @retval DB_UNREGISTER_UNKNOWN if the user doesn't exist.
 */
int Database::Unregister(std::string user_id, std::string password) {
	semaphore_wait();
	if (CorrectPassword(user_id, password) != 1) {
		semaphore_post();
		return DB_UNREGISTER_NOK;
	}

	semaphore_post();
	if (Logout(user_id, password) == DB_LOGOUT_NOK) {
		return DB_UNREGISTER_NOK;
	}
	semaphore_wait();

	int erased_password = ErasePassword(user_id);

	if (erased_password == -1) {
		semaphore_post();
		return DB_UNREGISTER_NOK;
	}

	if (erased_password == 0) {
		semaphore_post();
		return DB_UNREGISTER_OK;
	}

	if (erased_password == 2) {
		semaphore_post();
		return DB_UNREGISTER_UNKNOWN;
	}

	semaphore_post();
	return DB_UNREGISTER_NOK;
}

/**
 * @brief  Creates a new auction.
 * @param  user_id: The user's id.
 * @param  name: The name of the asset auctioned.
 * @param  password: The user's password.
 * @param  asset_fname: The path to the image of the asset.
 * @param  start_value: The starting value of the asset.
 * @param  timeactive: The time the auction will be active for.
 * @param  fsize: The size of the data file of the asset's image.
 * @param  data: The data of the asset's image.
 * @throws UserNotLoggedIn if the user isn't logged in.
 * @retval DB_OPEN_NOT_LOGGED_IN if the user isn't logged in
 * @retval DB_OPEN_CREATE_FAIL if the password is wrong, the directory, start
 * file or asset of the auction isn't properly created or the host isn't
 * properly registered.
 * @retval If successful returns the id of the newly created auction.
 */
int Database::Open(std::string user_id, std::string name, std::string password,
                   std::string asset_fname, std::string start_value,
                   std::string timeactive, size_t fsize, std::string data) {
	(void) fsize;
	semaphore_wait();
	if (CheckUserLoggedIn(user_id) != 0) {
		semaphore_post();
		throw UserNotLoggedIn();
		return DB_OPEN_NOT_LOGGED_IN;
	}
	if (CorrectPassword(user_id, password) != 1) {
		semaphore_post();
		return DB_OPEN_CREATE_FAIL;
	}
	uint32_t aid = 0;
	std::string new_aid;
	uint32_t n_new_aid;

	std::string dir_name = "ASDIR/AUCTIONS/";

	if (fs::is_empty(dir_name)) {
		aid = 1;
	} else {
		for (const auto &entry : fs::directory_iterator(dir_name)) {
			// Goes through the entire auctions directory in order to calculate
			// the newest auction id.
			new_aid = entry.path();
			new_aid.erase(new_aid.begin(), new_aid.end() - 3);
			n_new_aid = static_cast<uint32_t>(stoi(new_aid));

			if (aid <= n_new_aid) {
				aid = n_new_aid + 1;
			}
		}
	}

	if (aid > 999) {
		semaphore_post();
		return DB_OPEN_CREATE_FAIL;
	}

	std::string c_aid = convert_auction_id_to_str(aid);
	std::string a_dir_name = dir_name + c_aid;
	const char *a_dir_fname = a_dir_name.c_str();

	if (CreateAuctionDir(c_aid) == -1) {
		semaphore_post();
		return DB_OPEN_CREATE_FAIL;
	}

	if (CreateStartFile(c_aid, user_id, name, asset_fname, start_value,
	                    timeactive) == -1) {
		rmdir(a_dir_fname);
		semaphore_post();
		return DB_OPEN_CREATE_FAIL;
	}

	if (CreateAssetFile(c_aid, asset_fname, data) == -1) {
		rmdir(a_dir_fname);
		semaphore_post();
		return DB_OPEN_CREATE_FAIL;
	}

	if (RegisterHost(user_id, c_aid) == -1) {
		rmdir(a_dir_fname);
		semaphore_post();
		return DB_OPEN_CREATE_FAIL;
	}

	semaphore_post();
	return static_cast<int>(aid);
}

/**
 * @brief  Closes the auction.
 * @param  a_id: The auction's id.
 * @param  user_id: The user's id.
 * @param  password: The user's password.
 * @throws UserDoesNotExist if the user doesn't exist.
 * @throws UserNotLoggedIn if the user isn't logged in.
 * @throws IncorrectPassword if the password is incorrect.
 * @throws AuctionNotFound if the auction doesn't exist.
 * @throws AuctionNotOwnedByUser if the auction wasn't created by the user.
 * @throws AuctionAlreadyClosed if the auction was already closed.
 * @retval DB_CLOSE_NOK if theu user doesn't exist, is not logged in, the
 * password isn't correct, the auction doesn't exist, the auction wasn't created
 * by the user or there was an error in closing.
 * @retval DB_CLOSE_ENDED_ALREADY if the auction was already finished.
 * @retval DB_CLOSE_OK if the auction closes successfully.
 */
int Database::CloseAuction(std::string a_id, std::string user_id,
                           std::string password) {
	std::string user_id_dir = "ASDIR/USERS/" + user_id;
	const char *user_id_dirname = user_id_dir.c_str();
	semaphore_wait();
	if (CheckUserExisted(user_id_dirname) == -1) {
		semaphore_post();
		throw UserDoesNotExist();
		return DB_CLOSE_NOK;
	}
	if (CheckUserLoggedIn(user_id) != 0) {
		semaphore_post();
		throw UserNotLoggedIn();
		return DB_CLOSE_NOK;
	}
	if (CorrectPassword(user_id, password) != 1) {
		semaphore_post();
		throw IncorrectPassword();
		return DB_CLOSE_NOK;
	}
	if (CheckAuctionExists(a_id) == -1) {
		semaphore_post();
		throw AuctionNotFound();
		return DB_CLOSE_NOK;
	}
	if (CheckAuctionBelongs(a_id, user_id) == -1) {
		semaphore_post();
		throw AuctionNotOwnedByUser();
		return DB_CLOSE_NOK;
	}

	std::string dir_name = "ASDIR/AUCTIONS/" + a_id;
	dir_name += "/END";
	dir_name += a_id;
	dir_name += ".txt";
	if (CheckEndExists(dir_name.c_str()) == 0) {
		semaphore_post();
		throw AuctionAlreadyClosed();
		return DB_CLOSE_ENDED_ALREADY;
	}

	StartInfo start;
	if (GetStart(a_id, start) == -1) {
		semaphore_post();
		throw AuctionNotFound();
		return DB_CLOSE_NOK;
	};

	time_t fulltime;
	uint32_t start_time = start.current_time;
	uint32_t current_time = static_cast<uint32_t>(time(&fulltime));
	uint32_t time_passed = current_time - start_time;
	uint32_t timeactive = static_cast<uint32_t>(stol(start.timeactive));
	if (time_passed >= timeactive) {
		Close(a_id);
		semaphore_post();
		throw AuctionAlreadyClosed();
		return DB_CLOSE_ENDED_ALREADY;
	}
	int res = Close(a_id);

	semaphore_post();
	return res;
}

/**
 * @brief  Lists the auctions the user hosts.
 * @param  user_id: The user's id.
 * @throws AuctionNotFound if the auction doesn't exist.
 * @retval The list of the auctions the user hosts.
 */
AuctionList Database::MyAuctions(std::string user_id) {
	std::string dir_name = "ASDIR/USERS/" + user_id;
	dir_name += "/HOSTED";

	AuctionList result;
	AuctionListing auction;
	StartInfo start;
	time_t fulltime;

	semaphore_wait();

	if (fs::is_empty(dir_name)) {
		semaphore_post();
		return result;  // Returns empty list which means user hosts no auctions
	} else {
		for (const auto &entry : fs::directory_iterator(dir_name)) {
			// Goes through the Hosted directory in order obtain all the
			// auctions the user hosted, closing those necessary.

			std::string aid = entry.path();
			aid.erase(aid.end() - 4, aid.end());
			aid.erase(aid.begin(), aid.end() - 3);

			auction.a_id = aid;
			std::string aid_dir_name = "ASDIR/AUCTIONS/" + aid;
			aid_dir_name += "/END_";
			aid_dir_name += aid;
			aid_dir_name += ".txt";

			if (CheckEndExists(aid_dir_name.c_str()) == -1) {
				if (GetStart(aid, start) == -1) {
					semaphore_post();
					throw AuctionNotFound();
					return result;
				};
				uint32_t start_time = start.current_time;
				uint32_t current_time = static_cast<uint32_t>(time(&fulltime));
				uint32_t time_passed = current_time - start_time;
				uint32_t timeactive =
					static_cast<uint32_t>(stol(start.timeactive));

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

	std::sort(result.begin(), result.end(), CompareByAid);
	semaphore_post();
	return result;
}

/**
 * @brief  Lists the auctions the user bid on.
 * @param  user_id: The user's id.
 * @throws AuctionNotFound if the auction doesn't exist.
 * @retval The list of the auctions the user bid on.
 */
AuctionList Database::MyBids(std::string user_id) {
	std::string bidded_dir_name = "ASDIR/USERS/" + user_id;
	bidded_dir_name += "/BIDDED";

	AuctionList result;
	AuctionListing auction;
	StartInfo start;
	time_t fulltime;

	semaphore_wait();

	if (fs::is_empty(bidded_dir_name)) {
		semaphore_post();
		return result;  // Returns empty list which means user made no bids
	} else {
		for (const auto &entry : fs::directory_iterator(bidded_dir_name)) {
			// Goes through the Bidded directory in order obtain all the
			// auctions the user bid on, closing those necessary.

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
					semaphore_post();
					throw AuctionNotFound();
					return result;
				};
				uint32_t start_time = start.current_time;
				uint32_t current_time = static_cast<uint32_t>(time(&fulltime));
				uint32_t time_passed = current_time - start_time;
				uint32_t timeactive =
					static_cast<uint32_t>(stol(start.timeactive));

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

	std::sort(result.begin(), result.end(), CompareByAid);
	semaphore_post();
	return result;
}

/**
 * @brief  Lists all auctions.
 * @throws AuctionNotFound if the auction doesn't exist.
 * @retval The list containing every auction.
 */
AuctionList Database::List() {
	std::string Dir_name = "ASDIR/AUCTIONS";

	AuctionList result;
	AuctionListing auction;
	StartInfo start;
	time_t fulltime;

	if (fs::is_empty(Dir_name)) {
		semaphore_post();
		return result;  // Returns empty list which means no auctions were
		                // ever made
	} else {
		for (const auto &entry : fs::directory_iterator(Dir_name)) {
			// Goes through the Auctions directory in order obtain all the
			// auctions, closing those necessary.

			std::string aid = entry.path();
			aid.erase(aid.begin(), aid.end() - 3);

			auction.a_id = aid;
			std::string dir_name = "ASDIR/AUCTIONS/" + aid;
			dir_name += "/END_";
			dir_name += aid;
			dir_name += ".txt";

			if (CheckEndExists(dir_name.c_str()) == -1) {
				if (GetStart(aid, start) == -1) {
					semaphore_post();
					throw AuctionNotFound();
					return result;
				};
				uint32_t start_time = start.current_time;
				uint32_t current_time = static_cast<uint32_t>(time(&fulltime));
				uint32_t time_passed = current_time - start_time;
				uint32_t timeactive =
					static_cast<uint32_t>(stol(start.timeactive));

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

	std::sort(result.begin(), result.end(), CompareByAid);
	semaphore_post();
	return result;
}

/**
 * @brief  Shows the information about the auction's asset.
 * @param  a_id: The auction's id.
 * @throws AssetDoesNotExist if the asset doesn't exist.
 * @retval DB_SHOW_ASSET_ERROR if the auction has not asset.
 * @retval Otherwise shows the asset's info.
 */
AssetInfo Database::ShowAsset(std::string a_id) {
	AssetInfo asset;

	std::string asset_dir = GetAssetDir(a_id);

	semaphore_wait();
	if (asset_dir == "") {
		semaphore_post();
		throw AssetDoesNotExist();
		return DB_SHOW_ASSET_ERROR;
	}

	asset.fdata = GetAssetData(asset_dir);
	asset.fsize = (asset.fdata).size();

	asset_dir.erase(asset_dir.begin(), asset_dir.begin() + 25);
	asset.asset_fname = asset_dir;

	semaphore_post();
	return asset;
}

/**
 * @brief  The user places a bid on a particular auction.
 * @param  user_id: The user's id.
 * @param  password: The user's password.
 * @param  a_id: The auction's id.
 * @param  bid_value: The value of the bid placed.
 * @throws UserNotLoggedIn if the user isn't logged in.
 * @throws AuctionNotFound if the auction doesn't exist.
 * @throws BidOnSelf if the user attempts to bid on an auction they hosted.
 * @throws AuctionAlreadyClosed if the auction is already closed.
 * @throws LargerBidAlreadyExists if the bid's value is too low.
 * @retval DB_BID_NOK if the user isn't logged in, the password is wrong, the
 * auction doesn't exist, the user attempts to bid on an auction they hosted,
 * the auction is already closed, or the bid's value is too low.
 * @retval DB_BID_REFUSE if the bid isn't created successfully.
 * @retval DB_BID_ACCEPT if the bid is successfully created.
 */
int Database::Bid(std::string user_id, std::string password, std::string a_id,
                  std::string bid_value) {
	semaphore_wait();
	if (CheckUserLoggedIn(user_id) != 0) {
		semaphore_post();
		throw UserNotLoggedIn();
		return DB_BID_NOK;
	}
	if (CorrectPassword(user_id, password) != 1) {
		semaphore_post();
		return DB_BID_NOK;
	}
	if (CheckAuctionExists(a_id) == -1) {
		semaphore_post();
		throw AuctionNotFound();
		return DB_BID_NOK;
	}
	if (CheckAuctionBelongs(a_id, user_id) == 0) {
		semaphore_post();
		throw BidOnSelf();
		return DB_BID_NOK;
	}

	BidInfo bid;
	StartInfo start;
	long value = stol(bid_value);

	std::string end_dir_name = "ASDIR/AUCTIONS/" + a_id;
	end_dir_name += "/END_";
	end_dir_name += a_id;
	end_dir_name += ".txt";

	if (CheckEndExists(end_dir_name.c_str()) == 0) {
		semaphore_post();
		throw AuctionAlreadyClosed();
		return DB_BID_NOK;
	}

	if (GetStart(a_id, start) == -1) {
		semaphore_post();
		throw AuctionNotFound();
		return DB_BID_NOK;
	};

	time_t fulltime;
	uint32_t start_time = start.current_time;
	uint32_t current_time = static_cast<uint32_t>(time(&fulltime));
	uint32_t time_passed = current_time - start_time;
	uint32_t timeactive = static_cast<uint32_t>(stol(start.timeactive));
	if (time_passed >= timeactive) {
		Close(a_id);
		throw AuctionAlreadyClosed();
		return DB_BID_NOK;
	}

	std::string bid_dir_name = "ASDIR/AUCTIONS/" + a_id;
	bid_dir_name += "/BIDS/";

	std::string bid_fname;

	if (fs::is_empty(bid_dir_name)) {
		// If the value isn't greater than the starting value of the asset it's
		// not a correct bid.

		long old_value = stol(start.start_value);

		if (old_value >= value) {
			semaphore_post();
			throw LargerBidAlreadyExists();
			return DB_BID_NOK;
		}
	} else {
		for (const auto &entry : fs::directory_iterator(bid_dir_name)) {
			// If the value isn't greater than an existing bid it's not a
			// correct bid.

			bid_fname = entry.path();

			GetBid(bid_fname, bid);
			long old_value = stol(bid.value);

			if (old_value >= value) {
				semaphore_post();
				throw LargerBidAlreadyExists();
				return DB_BID_NOK;
			}
		}
	}

	if (RegisterBid(user_id, a_id) == -1) {
		semaphore_post();
		return DB_BID_REFUSE;
	}

	if (CreateBidFile(a_id, user_id, bid_value) == -1) {
		semaphore_post();
		return DB_BID_REFUSE;
	}

	semaphore_post();
	return DB_BID_ACCEPT;
}

/**
 * @brief  Shows the auction's information and the most recent 50 bids placed on
 * it.
 * @param  a_id: The auction's id.
 * @throws AuctionNotFound if the auction doesn't exist.
 * @retval The acutin's information and the most recent 50 bids on it.
 */
AuctionRecord Database::ShowRecord(std::string a_id) {
	BidInfo bid;
	time_t fulltime;
	StartInfo start;
	EndInfo end;
	uint32_t n = 0;
	AuctionRecord result;
	BidList list;

	semaphore_wait();
	if (GetStart(a_id, start) == -1) {
		semaphore_post();
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
	uint32_t current_time = static_cast<uint32_t>(time(&fulltime));
	uint32_t time_passed = current_time - start_time;
	uint32_t timeactive = static_cast<uint32_t>(stol(start.timeactive));

	std::string end_dir_name = "ASDIR/AUCTIONS/" + a_id;
	end_dir_name += "/END_";
	end_dir_name += a_id;
	end_dir_name += ".txt";

	if (CheckEndExists(end_dir_name.c_str()) == 0) {
		GetEnd(end_dir_name.c_str(), end);
		result.active = false;
		result.end_datetime = end.end_date;
		result.end_timeelapsed = end.end_time;

	} else if (time_passed >= timeactive) {
		Close(a_id);
		GetEnd(end_dir_name.c_str(), end);
		result.active = false;
		result.end_datetime = end.end_date;
		result.end_timeelapsed = end.end_time;

	} else {
		result.active = true;
	}

	std::string dir_name = "ASDIR/AUCTIONS/" + a_id;
	dir_name += "/BIDS";

	std::string bid_fname;
	for (const auto &entry : fs::directory_iterator(dir_name)) {
		n++;
		bid_fname = entry.path();
		GetBid(bid_fname, bid);
		list.push_back(bid);
	}

	std::sort(list.begin(), list.end(), CompareByValue);

	if (n > 50) {
		list.erase(list.begin(), list.end() - 50);
	}

	result.list = list;
	semaphore_post();
	return result;
}
