#ifndef __UTILS__
#define __UTILS__

#include <string>
#include <filesystem>
#include <fstream>

#include "verifications.hpp"

typedef struct {
	std::string year;
	std::string month;
	std::string day;
	std::string hours;
	std::string minutes;
	std::string seconds;
} datetime;

typedef struct {
	uint32_t bidder_UID;
	uint32_t bid_value;
	datetime bid_date_time;
	uint32_t bid_sec_time;
} bid;

// Thrown when the MessageID does not match what was expected
class FileException : public std::runtime_error {
   public:
	FileException() : std::runtime_error("Couldn't open file.") {}
};

// -----------------------------------
// | Extract date and time			  |
// -----------------------------------
std::string extractDate(datetime datetime);
std::string extractTime(datetime datetime);

// -----------------------------------
// | Convert types					 |
// -----------------------------------

uint32_t convert_user_id(std::string string);
uint32_t convert_auction_id(std::string string);
uint32_t convert_auction_value(std::string string);
std::string convert_password(std::string string);
std::string convert_date_to_str(datetime date);

// -----------------------------------
// | Reading and writing on files	 |
// -----------------------------------

void sendFile(int connection_fd, std::filesystem::path file_path);
void readAndSaveToFile(const int fd, const std::string &file_name, const size_t file_size);
uint32_t getFileSize(std::filesystem::path file_path);

#endif