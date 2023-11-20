#ifndef __VERIFICATIONS__
#define __VERIFICATIONS__

#include <string>

int verify_user_id(std::string user_id);
int verify_password(std::string password);
int verify_name(std::string name);
int verify_asset_fname(std::string asset_fname);
int verify_start_value(std::string start_value);
int verify_timeactive(std::string timeactive);
int verify_a_id(std::string a_id);
int verify_value(uint32_t value);

#endif