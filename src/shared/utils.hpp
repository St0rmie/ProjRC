#ifndef __UTILS__
#define __UTILS__

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "verifications.hpp"

typedef struct {
	std::string year;
	std::string month;
	std::string day;
	std::string hours;
	std::string minutes;
	std::string seconds;
} Datetime;

typedef struct {
	uint32_t bidder_UID;
	uint32_t bid_value;
	Datetime bid_date_time;
	uint32_t bid_sec_time;
} Bid;

// Thrown when the MessageID does not match what was expected
class FileException : public std::runtime_error {
   public:
	FileException() : std::runtime_error("[ERROR] Couldn't open file.") {}
};

// -----------------------------------
// | Extract date and time			  |
// -----------------------------------
std::string extractDate(Datetime datetime);
std::string extractTime(Datetime datetime);

// -----------------------------------
// | Convert types					 |
// -----------------------------------

uint32_t convert_user_id(std::string string);
std::string convert_user_id_to_str(uint32_t uid);
uint32_t convert_auction_id(std::string string);
std::string convert_auction_id_to_str(uint32_t aid);
uint32_t convert_auction_value(std::string string);
std::string convert_password(std::string string);
std::string convert_date_to_str(Datetime date);
Datetime convert_str_to_date(std::string str);

// -----------------------------------
// | Reading and writing on files	 |
// -----------------------------------

void sendFile(int connection_fd, std::filesystem::path file_path);
void readAndSaveToFile(const int fd, const std::string &file_name,
                       const size_t file_size);
void saveToFile(std::string file_name, std::string path, std::string file_data);
std::string readFromFile(std::string pathname);
long getFileSize(std::filesystem::path file_path);

#endif