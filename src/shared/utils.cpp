#include "utils.hpp"

#include "protocol.hpp"

// -----------------------------------
// | Extract date and time			 |
// -----------------------------------
std::string extractDate(datetime datetime) {
	std::stringstream out;
	out << datetime.year << "-" << datetime.month << "-" << datetime.day;
	return out.str();
};

std::string extractTime(datetime datetime) {
	std::stringstream out;
	out << datetime.hours << ":" << datetime.minutes << ":" << datetime.seconds;
	return out.str();
}

// -----------------------------------
// | Convert types					 |
// -----------------------------------

uint32_t convert_user_id(std::string string) {
	if (verify_user_id(string) == -1) {
		throw InvalidMessageException();
	}
	return std::stoi(string);
}

uint32_t convert_auction_id(std::string string) {
	if (verify_auction_id(string) == -1) {
		throw InvalidMessageException();
	}
	return std::stoi(string);
}

uint32_t convert_auction_value(std::string string) {
	uint32_t value = std::stoi(string);
	if (verify_value(value) == -1) {
		throw InvalidMessageException();
	}
	return value;
}

std::string convert_password(std::string string) {
	if (verify_password(string) == -1) {
		throw InvalidMessageException();
	}
	return string;
}

std::string convert_date_to_str(datetime date) {
	std::string date_str;
	date_str += date.year + ":" + date.month;
	date_str += ":" + date.day;
	date_str += " ";
	date_str += date.hours + ":" + date.minutes;
	date_str += ":" + date.seconds;
	return date_str;
}
