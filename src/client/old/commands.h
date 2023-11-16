#ifndef __COMMANDS__
#define __COMMANDS__

/* Command COMMANDes */
#define COMMAND_LOGIN "login"
#define COMMAND_CREATE_AUC "open"
#define COMMAND_CLOSE_AUC "close"
#define COMMAND_LIST_STARTED_AUC "myauctions"
#define COMMAND_LIST_STARTED_AUC_SHORT "ma"
#define COMMAND_LIST_BIDDED_AUC "mybids"
#define COMMAND_LIST_BIDDED_AUC_SHORT "mb"
#define COMMAND_LIST_ALL_AUC "list"
#define COMMAND_SHOW_ASSET "show_asset"
#define COMMAND_SHOW_ASSET_SHORT "sa"
#define COMMAND_BID "bid"
#define COMMAND_BID_SHORT "b"
#define COMMAND_SHOW_RECORD "show_record"
#define COMMAND_SHOW_RECORD_SHORT "sr"
#define COMMAND_LOGOUT "logout"
#define COMMAND_UNREGISTER "unregister"
#define COMMAND_EXIT "exit"

/* Functions */ 
int loginCommand();
int createAuction();
int closeAuction();
int listStartedAuctions();
int listBiddedAuctions();
int listAllAuctions();
int showAsset();
int bid();
int showRecord();
int logoutCommand();
int unregisterCommand();
int exit();

#endif