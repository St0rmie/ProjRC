#ifndef __CLIENT_STATE__
#define __CLIENT_STATE__

#define LOGGED_OUT -1

class ClientState {
   public:
	int _user_id = LOGGED_OUT;
	void login(int user_id);
	void logout();
	int isLoggedIn();
};

#endif