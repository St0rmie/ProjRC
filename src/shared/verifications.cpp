#include "verifications.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int verify_user_id(std::string user_id) {
	if (user_id.size() != 6) {
		return -1;
	}

	if (user_id == "000000") {
		return -1;
	}

	for (int i = 0; i < 6; i++) {
		if (!isdigit(user_id[i])) {
			return -1;
		}
	}

	return 0;
}

int verify_password(std::string password) {
	if (password.size() != 8) {
		return -1;
	}

	for (int i = 0; i < 8; i++) {
		if (isalnum(password[i]) == false) {
			return -1;
		}
	}

	return 0;
}

int verify_name(std::string name) {
	if (name.size() > 10) {
		return -1;
	}

	for (int i = 0; i < name.size(); i++) {
		if (isalnum(name[i]) == false) {
			return -1;
		}
	}

	return 0;
}

int check_fname_not_forbidden(std::string fname) {
	std::vector<std::string> forbidden = {"..", "/", "."};
	if (std::find(forbidden.begin(), forbidden.end(), fname) !=
	    forbidden.end()) {
		return 0;
	};
	return -1;
}

int verify_asset_fname(std::string asset_fname) {
	std::ifstream file;

	if (check_fname_not_forbidden(asset_fname) == -1) {
		return -1;
	}

	file.open("assets/" + asset_fname);

	if (file) {
		file.close();
		return 0;
	}

	else {
		file.close();
		return -1;
	}
}

int verify_start_value(std::string start_value) {
	if (start_value.size() > 6) {
		return -1;
	}

	for (int i = 0; i < start_value.size(); i++) {
		if (isdigit(start_value[i]) == false) {
			return -1;
		}
	}

	return 0;
}

int verify_timeactive(std::string timeactive) {
	if (timeactive.size() > 5) {
		return -1;
	}

	if (timeactive == "0") {
		return -1;
	}

	for (int i = 0; i < timeactive.size(); i++) {
		if (isdigit(timeactive[i]) == false) {
			return -1;
		}
	}

	return 0;
}

int verify_auction_id(std::string a_id) {
	if (a_id.size() != 3) {
		return -1;
	}

	for (int i = 0; i < 3; i++) {
		if (isdigit(a_id[i]) == false) {
			return -1;
		}
	}

	return 0;
}

int verify_value(uint32_t value) {
	if (value <= 0) {
		return -1;
	}

	return 0;
}

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