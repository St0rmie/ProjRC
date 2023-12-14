#include "utils.hpp"

#include "protocol.hpp"

// -----------------------------------
// | Extract date and time			 |
// -----------------------------------
std::string extractDate(Datetime datetime) {
	std::stringstream out;
	out << datetime.year << "-" << datetime.month << "-" << datetime.day;
	return out.str();
};

std::string extractTime(Datetime datetime) {
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

std::string convert_user_id_to_str(uint32_t uid) {
	char uid_c[7];
	sprintf(uid_c, "%06d", uid);
	std::string result = uid_c;
	return result;
}

uint32_t convert_auction_id(std::string string) {
	if (verify_auction_id(string) == -1) {
		throw InvalidMessageException();
	}
	return std::stoi(string);
}

std::string convert_auction_id_to_str(uint32_t aid) {
	char aid_c[4];
	sprintf(aid_c, "%03d", aid);
	std::string result = aid_c;
	return result;
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

std::string convert_date_to_str(Datetime date) {
	std::string date_str;
	date_str += date.year + "-" + date.month;
	date_str += "-" + date.day;
	date_str += " ";
	date_str += date.hours + ":" + date.minutes;
	date_str += ":" + date.seconds;
	return date_str;
}

Datetime convert_str_to_date(std::string str) {
	Datetime result;
	int year, month, day, hours, minutes, seconds;
	sscanf(str.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hours,
	       &minutes, &seconds);
	result.year = std::to_string(year);
	result.month = std::to_string(month);
	result.day = std::to_string(day);
	result.hours = std::to_string(hours);
	result.minutes = std::to_string(minutes);
	result.seconds = std::to_string(seconds);
	return result;
}

// -----------------------------------
// | Reading and writing on files	 |
// -----------------------------------

void sendFile(int connection_fd, std::filesystem::path file_path) {
	std::ifstream file(file_path, std::ios::in | std::ios::binary);
	if (!file) {
		std::cerr << "Error opening file: " << file_path << std::endl;
		throw FileException();
	}

	char buffer[SOCKET_BUFFER_LEN];
	while (file) {
		file.read(buffer, SOCKET_BUFFER_LEN);
		ssize_t bytes_read = (ssize_t) file.gcount();
		ssize_t bytes_sent = 0;
		while (bytes_sent < bytes_read) {
			ssize_t sent = write(connection_fd, buffer + bytes_sent,
			                     (size_t) (bytes_read - bytes_sent));
			if (sent < 0) {
				throw MessageSendException();
			}
			bytes_sent += sent;
		}
	}
}

void saveToFile(std::string file_name, std::string path,
                std::string file_data) {
	std::string full_path = path + file_name;
	std::ofstream file;
	file.open(full_path, std::ofstream::trunc);
	if (!file.good()) {
		throw FileException();
	}

	file << file_data << std::endl;

	file.close();
}

std::string readFromFile(std::string pathname) {
	std::ifstream file(pathname);
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

uint32_t getFileSize(std::filesystem::path file_path) {
	try {
		return (uint32_t) std::filesystem::file_size(file_path);
	} catch (...) {
		return -1;
	}
}
