#include "client.hpp"

void Client::login(int user_id) {
	this->_user_id = user_id;
}

void Client::logout() {
	this->_user_id = LOGGED_OUT;
}

int Client::isLoggedIn() {
	return _user_id > -1;
}
