#include "client_state.hpp"

void ClientState::login(int user_id) {
	this->_user_id = user_id;
}

void ClientState::logout() {
	this->_user_id = LOGGED_OUT;
}

int ClientState::isLoggedIn() {
	return _user_id > -1;
}
