#ifndef __VERIFICATIONS__
#define __VERIFICATIONS__

/**
 * @file verifications.hpp
 * @brief This file contains the declaration of functions responsible for
 * verifying the inputs.
 */

#include <string>

// All the functions responsible for verifying whther an input is correctly
// formatted.
int verify_user_id(std::string user_id);
int verify_password(std::string password);
int verify_name(std::string name);
int check_fname_not_forbidden(std::string fname);
int verify_asset_fname(std::string asset_fname);
int verify_start_value(std::string start_value);
int verify_timeactive(std::string timeactive);
int verify_auction_id(std::string a_id);
int verify_value(uint32_t value);
int verify_port_number(std::string &port);

#endif