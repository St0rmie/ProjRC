#include "verifications.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "config.hpp"

/**
 * @brief  Checks if the given user id fits the required parameters.
 * @param  user_id: The user id.
 * @retval -1 if it doesn't fit the parameters.
 * @retval 0 if it fits the parameters.
 */
int verify_user_id(std::string user_id) {
	if (user_id.size() != 6) {
		return -1;
	}

	if (user_id == "000000") {
		return -1;
	}

	for (size_t i = 0; i < 6; i++) {
		if (!isdigit(user_id[i])) {
			return -1;
		}
	}

	return 0;
}

/**
 * @brief  Checks if the given password fits the required parameters.
 * @param  password: The password.
 * @retval -1 if it doesn't fit the parameters.
 * @retval 0 if it fits the parameters.
 */
int verify_password(std::string password) {
	if (password.size() != 8) {
		return -1;
	}

	for (size_t i = 0; i < 8; i++) {
		if (isalnum(password[i]) == false) {
			return -1;
		}
	}

	return 0;
}

/**
 * @brief  Checks if the given name fits the required parameters.
 * @param  name: The name.
 * @retval -1 if it doesn't fit the parameters.
 * @retval 0 if it fits the parameters.
 */
int verify_name(std::string name) {
	if (name.size() > MAX_FILENAME_SIZE) {
		return -1;
	}
	for (size_t i = 0; i < name.size(); i++) {
		if (isprint(name[i]) == false) {
			return -1;
		}
	}

	return 0;
}

/**
 * @brief  Checks if the given file name isn't forbidden.
 * @param  fname: The file name.
 * @retval -1 if it's forbidden.
 * @retval 0 if it isn't forbidden.
 */
int check_fname_not_forbidden(std::string fname) {
	std::vector<std::string> forbidden = {"..", "/", "."};
	if (std::find(forbidden.begin(), forbidden.end(), fname) ==
	    forbidden.end()) {
		return 0;
	};
	return -1;
}

/**
 * @brief  Checks if the given asset path fits the required parameters.
 * @param  asset_path: The asset path.
 * @retval -1 if it doesn't fit the parameters.
 * @retval 0 if it fits the parameters.
 */
int verify_asset_fname(std::string asset_path) {
	std::ifstream file;
	std::string assetf_name =
		asset_path.substr(asset_path.find_last_of("/\\") + 1);

	if (check_fname_not_forbidden(assetf_name) == -1) {
		return -1;
	}

	file.open(asset_path);

	if (file) {
		file.close();
		return 0;
	} else {
		file.close();
		return -1;
	}
	return -1;
}

/**
 * @brief  Checks if the given start value fits the required parameters.
 * @param  start_value: The start value.
 * @retval -1 if it doesn't fit the parameters.
 * @retval 0 if it fits the parameters.
 */
int verify_start_value(std::string start_value) {
	if (start_value.size() > 6) {
		return -1;
	}

	for (size_t i = 0; i < start_value.size(); i++) {
		if (isdigit(start_value[i]) == false) {
			return -1;
		}
	}

	return 0;
}

/**
 * @brief  Checks if the given time active fits the required parameters.
 * @param  timeactive: The time active.
 * @retval -1 if it doesn't fit the parameters.
 * @retval 0 if it fits the parameters.
 */
int verify_timeactive(std::string timeactive) {
	if (timeactive.size() > 5) {
		return -1;
	}

	if (timeactive == "0") {
		return -1;
	}

	for (size_t i = 0; i < timeactive.size(); i++) {
		if (isdigit(timeactive[i]) == false) {
			return -1;
		}
	}

	return 0;
}

/**
 * @brief  Checks if the given auction id fits the required parameters.
 * @param  a_id: The auction id.
 * @retval -1 if it doesn't fit the parameters.
 * @retval 0 if it fits the parameters.
 */
int verify_auction_id(std::string a_id) {
	if (a_id.size() != 3) {
		return -1;
	}

	for (size_t i = 0; i < 3; i++) {
		if (isdigit(a_id[i]) == false) {
			return -1;
		}
	}

	return 0;
}

/**
 * @brief  Checks if the given value fits the required parameters.
 * @param  value: The value.
 * @retval -1 if it doesn't fit the parameters.
 * @retval 0 if it fits the parameters.
 */
int verify_value(uint32_t value) {
	if (value <= 0) {
		return -1;
	}

	return 0;
}

/**
 * @brief  Checks if the port number fits the required parameters.
 * @param  &port: The port.
 * @retval -1 if it doesn't fit the parameters.
 * @retval 0 if it fits the parameters.
 */
int verify_port_number(std::string &port) {
	for (char c : port) {
		if (!std::isdigit(static_cast<unsigned char>(c))) {
			return -1;
		}
	}

	int32_t parsed_port = std::stoi(port);
	if (parsed_port <= 0 || parsed_port > 65535) {
		return -1;
	}

	return 0;
}