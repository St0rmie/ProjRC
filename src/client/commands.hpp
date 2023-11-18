#ifndef __COMMANDS__
#define __COMMANDS__

#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>

#define MAX_COMMAND_SIZE 1024

/* Command COMMANDes */
#define COMMAND_LOGIN "login"
#define COMMAND_CREATE_AUC "open"
#define COMMAND_CLOSE_AUC "close"
#define COMMAND_LIST_STARTED_AUC "myauctions"
#define COMMAND_LIST_STARTED_AUC_ALIAS "ma"
#define COMMAND_LIST_BIDDED_AUC "mybids"
#define COMMAND_LIST_BIDDED_AUC_ALIAS "mb"
#define COMMAND_LIST_ALL_AUC "list"
#define COMMAND_SHOW_ASSET "show_asset"
#define COMMAND_SHOW_ASSET_ALIAS "sa"
#define COMMAND_BID "bid"
#define COMMAND_BID_ALIAS "b"
#define COMMAND_SHOW_RECORD "show_record"
#define COMMAND_SHOW_RECORD_ALIAS "sr"
#define COMMAND_LOGOUT "logout"
#define COMMAND_UNREGISTER "unregister"
#define COMMAND_EXIT "exit"

/* Classes */

class CommandHandler{
   protected:
        CommandHandler(const char* name,
                        std::optional<const char*> alias,
                        const char* description)
                    : _name{name},
                    _alias{alias},
                    _description{description} {}

   public:
        const char* _name;
        const std::optional<const char*> _alias;
        const char* _description;
        virtual void handle(std::string name) = 0;

};

class CommandManager{
    private:
        std::unordered_map<std::string, std::shared_ptr<CommandHandler>> handlers;
    
    public:
        void registerCommand(std::shared_ptr<CommandHandler> handler);
        void waitCommand();
};

class LoginCommand : public CommandHandler {
    virtual void handle(std::string name);

    public:
        LoginCommand(): CommandHandler("login",std::nullopt,"Log In for User.") {}
};


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