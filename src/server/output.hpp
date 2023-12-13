#ifndef __SRV_OUTPUT__
#define __SRV_OUTPUT__

#include <string>

#include "server.hpp"

void printError(std::string message);
void printInfo(std::string message);

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