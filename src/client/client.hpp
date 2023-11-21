#ifndef __CLIENT__
#define __CLIENT__

#define LOGGED_OUT -1

class Client {
   public:
	int _user_id = LOGGED_OUT;
	void login(int user_id);
	void logout();
	int isLoggedIn();
};

#endif