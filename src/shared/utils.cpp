#include "utils.hpp"

#include "protocol.hpp"

// -----------------------------------
// | Extract date and time			 |
// -----------------------------------

/**
 * @brief  Recieves a struct containing the date and extracts it.
 * @param  datetime: The struct containing the date and time.
 * @retval A string containing the formated date
 */
std::string extractDate(Datetime datetime) {
	std::stringstream out;
	out << datetime.year << "-" << datetime.month << "-" << datetime.day;
	return out.str();
};

/**
 * @brief  Recieves a struct containing the time and extracts it.
 * @param  datetime: The struct containing the date and time.
 * @retval A string containing the formated time
 */
std::string extractTime(Datetime datetime) {
	std::stringstream out;
	out << datetime.hours << ":" << datetime.minutes << ":" << datetime.seconds;
	return out.str();
}

// -----------------------------------
// | Convert types					 |
// -----------------------------------

/**
 * @brief  Converts a user id in the form of a string into a uint32_t.
 * @param  string: The user id to convert.
 * @throws InvalidMessageException if the user id isn't valid.
 * @retval The user id as a uint32_t.
 */
uint32_t convert_user_id(std::string string) {
	if (verify_user_id(string) == -1) {
		throw InvalidMessageException();
	}
	return (static_cast<uint32_t>(std::stol(string)));
}

/**
 * @brief  Converts a user id in the form of a uint32_t into a string.
 * @param  uid: The user id to convert.
 * @retval The user id as a string.
 */
std::string convert_user_id_to_str(uint32_t uid) {
	char uid_c[7];
	sprintf(uid_c, "%06d", uid);
	std::string result = uid_c;
	return result;
}

/**
 * @brief  Converts an auction id in the form of a string into a uint32_t.
 * @param  string: The auction id to convert.
 * @throws InvalidMessageException if the auction id isn't valid.
 * @retval The auction id as a uint32_t.
 */
uint32_t convert_auction_id(std::string string) {
	if (verify_auction_id(string) == -1) {
		throw InvalidMessageException();
	}
	return (static_cast<uint32_t>(std::stoi(string)));
}

/**
 * @brief  Converts an auction id in the form of a uint32_t into a string.
 * @param  aid: The auction id to convert.
 * @retval The auction id as a string.
 */
std::string convert_auction_id_to_str(uint32_t aid) {
	char aid_c[4];
	sprintf(aid_c, "%03d", aid);
	std::string result = aid_c;
	return result;
}

/**
 * @brief  Converts a value in the form of a string into a uint32_t.
 * @param  string: The value to convert.
 * @throws InvalidMessageException if the value isn't valid.
 * @retval The value as a uint32_t.
 */
uint32_t convert_auction_value(std::string string) {
	uint32_t value = static_cast<uint32_t>(std::stol(string));
	if (verify_value(value) == -1) {
		throw InvalidMessageException();
	}
	return value;
}

/**
 * @brief  Verifies the password.
 * @param  string: The password to verify.
 * @throws InvalidMessageException if the ppassword isn't valid.
 * @retval The verified password.
 */
std::string convert_password(std::string string) {
	if (verify_password(string) == -1) {
		throw InvalidMessageException();
	}
	return string;
}

/**
 * @brief  Converts a date in the form of a Datetime into a string.
 * @param  date: The date to convert.
 * @retval The date as a string.
 */
std::string convert_date_to_str(Datetime date) {
	std::string date_str;
	date_str += date.year + "-" + date.month;
	date_str += "-" + date.day;
	date_str += " ";
	date_str += date.hours + ":" + date.minutes;
	date_str += ":" + date.seconds;
	return date_str;
}

/**
 * @brief  Converts a date in the form of a string into a Datetime.
 * @param  str: The date to convert.
 * @retval The date as a Datetime.
 */
Datetime convert_str_to_date(std::string str) {
	Datetime result;
	int year, month, day, hours, minutes, seconds;
	char buffer[10];
	sscanf(str.c_str(), "%04d-%02d-%02d %02d:%02d:%02d", &year, &month, &day,
	       &hours, &minutes, &seconds);
	sprintf(buffer, "%04d", year);
	result.year = std::string(buffer, 4);
	sprintf(buffer, "%02d", month);
	result.month = std::string(buffer, 2);
	sprintf(buffer, "%02d", day);
	result.day = std::string(buffer, 2);
	sprintf(buffer, "%02d", hours);
	result.hours = std::string(buffer, 2);
	sprintf(buffer, "%02d", minutes);
	result.minutes = std::string(buffer, 2);
	sprintf(buffer, "%02d", seconds);
	result.seconds = std::string(buffer, 2);
	return result;
}

// -----------------------------------
// | Reading and writing on files	 |
// -----------------------------------

/**
 * @brief  Creates a file and writes data to it, if one with the same name
 * exists it writes to it.
 * @param  file_name: The name of the file.
 * @param  path: The directory in which the file is or will be.
 * @param  file_data: The data to be written.
 * @throws FileException if the file can't be opened or created.
 * @retval None
 * */
void saveToFile(std::string file_name, std::string path,
                std::string file_data) {
	std::string full_path = path + file_name;
	std::ofstream file;
	std::cout << full_path << std::endl;
	file.open(full_path, std::ofstream::trunc);
	if (!file.good()) {
		throw FileException();
	}

	file << file_data;

	file.close();
}

/**
 * @brief  Reads the data from a file.
 * @param  pathname: The path to the file.
 * @retval The data retrieved.
 */
std::string readFromFile(std::string pathname) {
	std::ifstream file(pathname);
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

/**
 * @brief  Obtains the file's size.
 * @param  file_path: The path to the file.
 * @retval The file's size.
 */
long getFileSize(std::filesystem::path file_path) {
	try {
		return static_cast<long long>(std::filesystem::file_size(file_path));
	} catch (...) {
		return -1;
	}
}
