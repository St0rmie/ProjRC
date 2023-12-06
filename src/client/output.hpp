#ifndef __OUTPUT__
#define __OUTPUT__

#include "shared/protocol.hpp"

void printRecord(std::string aid, ServerShowRecord message);
void printOpenAuction(ServerOpenAuction message);
void printCloseAuction(ClientCloseAuction message);

#endif