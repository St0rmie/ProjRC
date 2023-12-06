#ifndef __OUTPUT__
#define __OUTPUT__

#include "shared/protocol.hpp"

void printRecord(std::string aid, ServerShowRecord message);
void printOpenAuction(ServerOpenAuction message);
void printCloseAuction(ClientCloseAuction message);
void printShowAsset(ClientShowAsset message);
void printBid(ClientBid message);
void printError(std::string str);
void printSuccess(std::string success_description);

#endif