#ifndef __COMMANDS__
#define __COMMANDS__

/**
 * @file commands.hpp
 * @brief Contains the declaration of client command-related classes and
 * functions.
 */

#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "client.hpp"

// Maximum size of a command input
#define MAX_COMMAND_SIZE 1024

// -----------------------------------
// | Command name and aliases		 |
// -----------------------------------

#define COMMAND_LOGIN                  "login"
#define COMMAND_CREATE_AUC             "open"
#define COMMAND_CLOSE_AUC              "close"
#define COMMAND_LIST_STARTED_AUC       "myauctions"
#define COMMAND_LIST_STARTED_AUC_ALIAS "ma"
#define COMMAND_LIST_BIDDED_AUC        "mybids"
#define COMMAND_LIST_BIDDED_AUC_ALIAS  "mb"
#define COMMAND_LIST_ALL_AUC           "list"
#define COMMAND_SHOW_ASSET             "show_asset"
#define COMMAND_SHOW_ASSET_ALIAS       "sa"
#define COMMAND_BID                    "bid"
#define COMMAND_BID_ALIAS              "b"
#define COMMAND_SHOW_RECORD            "show_record"
#define COMMAND_SHOW_RECORD_ALIAS      "sr"
#define COMMAND_LOGOUT                 "logout"
#define COMMAND_UNREGISTER             "unregister"
#define COMMAND_EXIT                   "exit"

// ---------------------------------------
// | Command Manager and Handler classes |
// ---------------------------------------

class CommandHandler {
   protected:
	CommandHandler(const char *name, std::optional<const char *> alias,
	               const char *usage, const char *description)
		: _name{name},
		  _alias{alias},
		  _usage{usage},
		  _description{description} {}

   public:
	const char *_name;
	const std::optional<const char *> _alias;
	const char *_usage;
	const char *_description;
	virtual void handle(std::string name, Client &client) = 0;
};

class CommandManager {
   private:
	std::unordered_map<std::string, std::shared_ptr<CommandHandler>> handlers;

   public:
	void registerCommand(std::shared_ptr<CommandHandler> handler);
	void waitCommand(Client &client);
	void printHelp();
};

// -----------------------------------
// | Commands classes				 |
// -----------------------------------

/**
 * @brief    Command to log in as a user.
 */
class LoginCommand : public CommandHandler {
	virtual void handle(std::string name, Client &client);

   public:
	LoginCommand()
		: CommandHandler("login", std::nullopt, "login <UID> <password>",
	                     "Log In as user UID.") {}
};

/**
 * @brief    Command to open a auction.
 */
class OpenAuctionCommand : public CommandHandler {
	virtual void handle(std::string name, Client &client);

   public:
	OpenAuctionCommand()
		: CommandHandler("open", std::nullopt,
	                     "open <name> <asset_fname> <start_value> <timeactive>",
	                     "Open a new auction for the logged in user.") {}
};

/**
 * @brief    Command to close a auction.
 */
class CloseAuctionCommand : public CommandHandler {
	virtual void handle(std::string name, Client &client);

   public:
	CloseAuctionCommand()
		: CommandHandler("close", std::nullopt, "close <AID>",
	                     "Close ongoing auction.") {}
};

/**
 * @brief    Command to list auctions started by a user.
 */
class ListStartedAuctionsCommand : public CommandHandler {
	virtual void handle(std::string name, Client &client);

   public:
	ListStartedAuctionsCommand()
		: CommandHandler("myauctions", "ma", "myauctions",
	                     "List auctions started by the logged in User") {}
};

/**
 * @brief    Command to list auctions bidded by a user.
 */
class ListBiddedAuctionsCommand : public CommandHandler {
	virtual void handle(std::string name, Client &client);

   public:
	ListBiddedAuctionsCommand()
		: CommandHandler("mybids", "mb", "mybids",
	                     "List auctions in which the logged in User bidded.") {}
};

/**
 * @brief    Command to list all auctions on the auction server.
 */
class ListAllAuctionsCommand : public CommandHandler {
	virtual void handle(std::string name, Client &client);

   public:
	ListAllAuctionsCommand()
		: CommandHandler("list", "l", "list", "List all auctions.") {}
};

/**
 * @brief    Command to show asset of an auction.
 */
class ShowAssetCommand : public CommandHandler {
	virtual void handle(std::string name, Client &client);

   public:
	ShowAssetCommand()
		: CommandHandler("show_asset", "sa", "show_asset <AID>",
	                     "Retrieve asset file of an auction.") {}
};

/**
 * @brief    Command to , as a user, bid on an auction.
 */
class BidCommand : public CommandHandler {
	virtual void handle(std::string name, Client &client);

   public:
	BidCommand()
		: CommandHandler("bid", "b", "bid <AID> <value>",
	                     "Place a bid on an auction.") {}
};

/**
 * @brief    Command to show record of an auction.
 */
class ShowRecordCommand : public CommandHandler {
	virtual void handle(std::string name, Client &client);

   public:
	ShowRecordCommand()
		: CommandHandler("show_record", "sr", "show_record <AID>",
	                     "Show entire record (start info. , bids, end info. "
	                     "...) of an auction.") {}
};

/**
 * @brief    Command to logout from a user.
 */
class LogoutCommand : public CommandHandler {
	virtual void handle(std::string name, Client &client);

   public:
	LogoutCommand()
		: CommandHandler("logout", std::nullopt, "logout",
	                     "Log Out of a User.") {}
};

/**
 * @brief    Command to unregister the logged in user.
 */
class UnregisterCommand : public CommandHandler {
	virtual void handle(std::string name, Client &client);

   public:
	UnregisterCommand()
		: CommandHandler("unregister", std::nullopt, "unregister",
	                     "Unregister and logout of the logged in User.") {}
};

/**
 * @brief    Command to exit of the client application.
 */
class ExitCommand : public CommandHandler {
	virtual void handle(std::string name, Client &client);

   public:
	ExitCommand()
		: CommandHandler("exit", std::nullopt, "exit",
	                     "Exit the application.") {}
};

/**
 * @brief    Command to print all the registered commands information.
 */
class HelpCommand : public CommandHandler {
	virtual void handle(std::string name, Client &client);
	CommandManager &_manager;

   public:
	HelpCommand(CommandManager &manager)
		: CommandHandler("help", std::nullopt, "help",
	                     "Show information about all registered commands "
	                     "(name,alias,usage,description)."),
		  _manager(manager) {}
};

// -----------------------------------
// | Registering the commands		 |
// -----------------------------------

void registerCommands(CommandManager &manager);

#endif