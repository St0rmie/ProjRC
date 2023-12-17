#ifndef __SRV_OUTPUT__
#define __SRV_OUTPUT__

/**
 * @file output.hpp
 * @brief Header file for a output instance.
 */

#include <string>

#include "server.hpp"

// All the functions resposinble for printing.
void printError(std::string message);
void printInfo(std::string message, int tab_level);

std::string hidePassword(std::string password);

void printAddressIncomingRequest(Address &addr_from);
void printInLoginRequest(ClientLoginUser request);
void printInLogoutRequest(ClientLogout request);
void printInUnregisterRequest(ClientUnregister request);
void printInListAllRequest(ClientListAllAuctions request);
void printInListBiddedRequest(ClientListBiddedAuctions request);
void printInListStartedRequest(ClientListStartedAuctions request);
void printInShowRecordRequest(ClientShowRecord request);
void printInOpenAuctionRequest(ClientOpenAuction request);
void printInCloseAuctionRequest(ClientCloseAuction request);
void printInShowAssetRequest(ClientShowAsset request);
void printInBidRequest(ClientBid request);
#endif