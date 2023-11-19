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

class CreateAuctionCommand : public CommandHandler {
    virtual void handle(std::string name);
    
    public:
        CreateAuctionCommand(): CommandHandler("open",std::nullopt,"Open a new auction for User.") {}
};

class CloseAuctionCommand : public CommandHandler {
    virtual void handle(std::string name);
    
    public:
        CloseAuctionCommand(): CommandHandler("close",std::nullopt,"Close ongoing auction for User.") {}
};

class ListStartedAuctionsCommand : public CommandHandler {
    virtual void handle(std::string name);
    
    public:
        ListStartedAuctionsCommand(): CommandHandler("myauctions","ma","List auctions started by this User or where they placed a bid.") {}
};

class ListBiddedAuctionsCommand : public CommandHandler {
    virtual void handle(std::string name);
    
    public:
        ListBiddedAuctionsCommand(): CommandHandler("mybids","mb","List auctions for which this User made a bid.") {}
};

class ListAllAuctionsCommand : public CommandHandler {
    virtual void handle(std::string name);
    
    public:
        ListAllAuctionsCommand(): CommandHandler("list","l","List all active auction.") {}
};

class ShowAssetCommand : public CommandHandler {
    virtual void handle(std::string name);
    
    public:
        ShowAssetCommand(): CommandHandler("show_asset","sa","Show asset on sale in the auction.") {}
};

class BidCommand : public CommandHandler {
    virtual void handle(std::string name);
    
    public:
        BidCommand(): CommandHandler("bid","b","Place a bid for auction.") {}
};

class ShowRecordCommand : public CommandHandler {
    virtual void handle(std::string name);
    
    public:
        ShowRecordCommand(): CommandHandler("show_record", "sr","Show record of auction.") {}
};

class LogoutCommand : public CommandHandler {
    virtual void handle(std::string name);

    public:
        LogoutCommand(): CommandHandler("logout",std::nullopt,"Log Out for User.") {}
};

class UnregisterCommand : public CommandHandler {
    virtual void handle(std::string name);

    public:
        UnregisterCommand(): CommandHandler("unregister",std::nullopt,"Unregister the logged in User.") {}
};

class ExitCommand : public CommandHandler {
    virtual void handle(std::string name);

    public:
        ExitCommand(): CommandHandler("exit",std::nullopt,"Exit the application.") {}
};

#endif