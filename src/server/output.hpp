#ifndef __SRV_OUTPUT__
#define __SRV_OUTPUT__

#include <string>

#include "server.hpp"

void printError(std::string message);
void printInfo(std::string message);

void printAddressIncomingRequest(Address &addr_from);
#endif