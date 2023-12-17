/**
 * @file commands.cpp
 * @brief Implementation of the CommandManager class and its associated command
 * handlers and functions.
 *
 * This file contains the implementation of the CommandManager class, which is
 * responsible for registering and handling commands. It also includes the
 * implementation of various command handlers for each command that the user can
 * execute.
 */

#include "commands.hpp"

#include <readline/history.h>
#include <readline/readline.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>

#include "client.hpp"
#include "output.hpp"
#include "shared/protocol.hpp"
#include "shared/verifications.hpp"

/**
 * @brief  Registers a command handler onto the command manager.
 * @param  handler: a shared pointer to the command handler to be registered
 * @retval None
 */
void CommandManager::registerCommand(std::shared_ptr<CommandHandler> handler) {
	this->handlers.insert({handler->_name, handler});
	// If the command has an alias make a new registry for the handler under the
	// alias
	if (handler->_alias) {
		this->handlers.insert({*handler->_alias, handler});
	}
}

/**
 * @brief  Waits for a command to be inputted by the user onto the terminal and
 * calls the respective handler.
 * @param  &client: reference to the client instance
 * @retval None
 */
void CommandManager::waitCommand(Client &client) {
	std::string line_base;
	if (client.isLoggedIn()) {  // Show user id if logged in
		line_base += "[";
		line_base += std::to_string(client.getLoggedInUser());
		line_base += "] > ";
	} else {  // Show default prompt
		line_base += "> ";
	}

	// Waits for user input
	char *input = readline(line_base.c_str());

	// Adds the input to the history
	add_history(input);
	std::string line(input);
	free(input);

	// Splits the line between command name and arguments
	size_t splitIndex = line.find(' ');
	std::string commandName;
	if (splitIndex == std::string::npos) {
		// No arguments
		commandName = line;
		line = "";
	} else {
		// Has arguments
		commandName = line.substr(0, splitIndex);
		line.erase(0, splitIndex + 1);
	}

	// Checks if the command is valid
	if (commandName.length() == 0) {
		return;
	}

	// Finds the proper handler for the command
	auto handler = this->handlers.find(commandName);
	if (handler == this->handlers.end()) {
		printError(
			"Unknown Command. Type \"help\" for the list of "
			"commands available.");
		return;
	}

	// Calls the handler of the command
	handler->second->handle(line, client);
}

/**
 * @brief  Prints the help message with the list of commands available.
 * @retval None
 */
void CommandManager::printHelp() {
	std::cout << "[HELP] Available commands:" << std::endl << std::left;

	// Iterates through the handlers and prints the help message for each
	for (auto it = this->handlers.begin(); it != this->handlers.end(); ++it) {
		auto handler = it->second;
		std::string ss;
		ss += "\n\tName: ";
		ss += handler->_name;
		ss += "\n";
		if (handler->_alias) {
			ss += "\tAlias: ";
			ss += handler->_alias.value();
			ss += "\n";
		}
		ss += "\tUsage: ";
		ss += handler->_usage;
		ss += "\n";
		ss += "\tDescription: ";
		ss += handler->_description;
		ss += "\n";
		std::cout << ss;
	}
}

/**
 * @brief Handles the login command.
 * @param  args: arguments following the command
 * @param  &client: reference to the client instance
 * @retval None
 */
void LoginCommand::handle(std::string args, Client &client) {
	// Parsing the arguments
	std::stringstream ss(args);
	std::vector<std::string> parsed_args;
	std::string arg;
	// Extract arguments to a vector
	while (ss >> arg) {
		parsed_args.push_back(arg);
	}
	// Check if the number of arguments is correct
	if (parsed_args.size() != 2) {
		printError("Wrong number of arguments");
		return;
	}

	// Check if the user is already logged in
	if (client.isLoggedIn() == true) {
		printError("Already Logged In. Please logout first.");
		return;
	}

	// Defining the arguments extracted
	std::string user_id = parsed_args[0];
	std::string password = parsed_args[1];

	// Verifying parameters
	if (verify_user_id(user_id) == -1) {
		printError("Incorrect user id.");
		return;
	}
	if (verify_password(password) == -1) {
		printError("Incorrect password.");
		return;
	}

	// Fill and send message
	ClientLoginUser message_out;
	message_out.user_id = convert_user_id(user_id);
	message_out.password = convert_password(password);

	// Message to receive
	ServerLoginUser message_in;
	if (client.sendUdpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	}

	// Check status of the received message
	switch (message_in.status) {
		case ServerLoginUser::status::OK:
			client.login(convert_user_id(user_id), convert_password(password));
			printSuccess("Sucessfully logged in as " +
			             std::to_string(client.getLoggedInUser()));
			break;

		case ServerLoginUser::status::NOK:
			printError("Couldn't login.");
			break;

		case ServerLoginUser::status::REG:
			client.login(convert_user_id(user_id), convert_password(password));
			printSuccess("Registered user " +
			             std::to_string(client.getLoggedInUser()));
			break;

		case ServerLoginUser::status::ERR:
			printError("Wrong format sent.");
			break;

		default:
			throw InvalidMessageException();
	}
}

/**
 * @brief Handles the logout command.
 * @param  args: arguments following the command
 * @param  &client: reference to the client instance
 * @retval None
 */
void LogoutCommand::handle(std::string args, Client &client) {
	// Verify number of arguments
	if (args.length() > 0) {
		printError("Wrong number of arguments");
		return;
	}

	// Verify if the user is logged in
	if (client.isLoggedIn() == false) {
		printError("Not logged in. Please login first.");
		return;
	}

	// Fill and send message
	ClientLogout message_out;
	message_out.user_id = client.getLoggedInUser();
	message_out.password = client.getPassword();

	// Message to receive
	ServerLogout message_in;
	if (client.sendUdpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	}

	// Check status of the received message
	switch (message_in.status) {
		case ServerLogout::status::OK:
			client.logout();
			printSuccess("Sucessfully logged out");
			break;

		case ServerLogout::status::NOK:
			printError("Couldn't logout.");
			break;

		case ServerLogout::status::UNR:
			printError("Unregistered user.");
			break;

		case ServerLogout::status::ERR:
			printError("Wrong format sent.");
			break;

		default:
			throw InvalidMessageException();
	}
}

/**
 * @brief Handles the unregister command.
 * @param  args: arguments following the command
 * @param  &client: reference to the client instance
 * @retval None
 */
void UnregisterCommand::handle(std::string args, Client &client) {
	// Verify number of arguments
	if (args.length() > 0) {
		printError("Wrong number of arguments");
		return;
	}

	// Verify if the user is logged in
	if (client.isLoggedIn() == false) {
		printError("Not logged in. Please login first.");
		return;
	}

	// Fill and send message
	ClientUnregister message_out;
	message_out.user_id = client.getLoggedInUser();
	message_out.password = client.getPassword();

	// Message to receive
	ServerUnregister message_in;
	if (client.sendUdpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	}

	// Check status of the received message
	switch (message_in.status) {
		case ServerUnregister::status::OK:
			client.logout();
			printSuccess("Sucessfully unregister.");
			break;

		case ServerUnregister::status::NOK:
			printError("Not logged in, hence couldn't unregister.");
			break;

		case ServerUnregister::status::UNR:
			printError("Unregistered user.");
			break;

		case ServerUnregister::status::ERR:
			printError("Wrong format sent.");
			break;

		default:
			throw InvalidMessageException();
	}
}

/**
 * @brief Handles the list started auctions command.
 * @param  args: arguments following the command
 * @param  &client: reference to the client instance
 * @retval None
 */
void ListStartedAuctionsCommand::handle(std::string args, Client &client) {
	// Verify number of arguments
	if (args.length() > 0) {
		printError("Wrong number of arguments");
		return;
	}

	// Verify if the user is logged in
	if (client.isLoggedIn() == false) {
		printError("Not logged in. Please login first.");
		return;
	}

	// Fill and send message
	ClientListStartedAuctions message_out;
	message_out.user_id = client.getLoggedInUser();

	// Message to receive
	ServerListStartedAuctions message_in;
	if (client.sendUdpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	}
	std::string message_ok =
		"Listing \nAuctions started by user " + client.getLoggedInUser();
	message_ok += ":";

	// Check status
	switch (message_in.status) {
		case ServerListStartedAuctions::status::OK:
			printSuccess(message_ok);
			for (std::string auc : message_in.auctions) {
				std::cout << "\t" << auc << std::endl;
			}
			break;

		case ServerListStartedAuctions::status::NOK:
			printError("User doesn't have ongoing auctions.");
			break;

		case ServerListStartedAuctions::status::NLG:
			printError("User not logged in.");
			break;

		case ServerListStartedAuctions::status::ERR:
			printError("Wrong format sent.");
			break;

		default:
			throw InvalidMessageException();
	}
}

/**
 * @brief Handles the list mybids command.
 * @param  args: arguments following the command
 * @param  &client: reference to the client instance
 * @retval None
 */
void ListBiddedAuctionsCommand::handle(std::string args, Client &client) {
	if (args.length() > 0) {
		printError("Wrong number of arguments");
		return;
	}

	if (client.isLoggedIn() == false) {
		printError("Not logged in. Please login first.");
		return;
	}

	// Populate and send packet
	ClientListBiddedAuctions message_out;
	message_out.user_id = client.getLoggedInUser();

	// Message to receive
	ServerListBiddedAuctions message_in;
	if (client.sendUdpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	}

	std::string message_ok =
		"Listing \nAuctions bidded by user " + client.getLoggedInUser();
	message_ok += ":";

	// Check status of the received message
	switch (message_in.status) {
		case ServerListBiddedAuctions::status::OK:;
			printSuccess(message_ok);
			for (std::string auc : message_in.auctions) {
				std::cout << "\t" << auc << std::endl;
			}
			break;

		case ServerListBiddedAuctions::status::NOK:
			printError("User didn't bid on any auctions.");
			break;

		case ServerListBiddedAuctions::status::NLG:
			printError("User not logged in.");
			break;

		case ServerListBiddedAuctions::status::ERR:
			printError("Wrong format sent.");
			break;

		default:
			throw InvalidMessageException();
	}
}

/**
 * @brief Handles the list all auctions command.
 * @param  args: arguments following the command
 * @param  &client: reference to the client instance
 * @retval None
 */
void ListAllAuctionsCommand::handle(std::string args, Client &client) {
	// Verify the number of arguments
	if (args.length() > 0) {
		printError("Wrong number of arguments");
		return;
	}

	// Fill and send message
	ClientListAllAuctions message_out;

	// Message to receive
	ServerListAllAuctions message_in;
	if (client.sendUdpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	}

	// Check status of the received message
	switch (message_in.status) {
		case ServerListAllAuctions::status::OK:;
			printSuccess("Listing \nAll auctions registered on the server:");
			for (std::string auc : message_in.auctions) {
				std::cout << "\t" << auc << std::endl;
			}
			break;

		case ServerListAllAuctions::status::NOK:
			printError("There aren't any auctions on the system yet.");
			break;

		case ServerListAllAuctions::status::ERR:
			printError("Wrong format sent.");
			break;

		default:
			throw InvalidMessageException();
	}
}

/**
 * @brief Handles the list started auctions command.
 * @param  args: arguments following the command
 * @param  &client: reference to the client instance
 * @retval None
 */
void ShowRecordCommand::handle(std::string args, Client &client) {
	// Parsing the arguments
	std::stringstream ss(args);
	std::vector<std::string> parsed_args;
	std::string arg;

	// Extract arguments to a vector
	while (ss >> arg) {
		parsed_args.push_back(arg);
	}

	// Check if the number of arguments is correct
	if (parsed_args.size() != 1) {
		printError("Wrong number of arguments");
		return;
	}

	// Defining the arguments extracted
	std::string a_id = parsed_args[0];

	// Verify the arguments
	if (verify_auction_id(a_id) == -1) {
		printError("Incorrect AID.");
		return;
	}

	// Fill and send message
	ClientShowRecord message_out;
	message_out.auction_id = convert_auction_id(a_id);

	// Message to receive
	ServerShowRecord message_in;
	if (client.sendUdpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	}

	// Check status of the received message
	switch (message_in.status) {
		case ServerShowRecord::status::OK:;
			printRecord(a_id, message_in);
			break;

		case ServerShowRecord::status::NOK:
			printError("There aren't any auctions on the system yet.");
			break;

		case ServerShowRecord::status::ERR:
			printError("Wrong format sent.");
			break;

		default:
			throw InvalidMessageException();
	}
}

/**
 * @brief Handles the open auction command.
 * @param  args: arguments following the command
 * @param  &client: reference to the client instance
 * @retval None
 */
void OpenAuctionCommand::handle(std::string args, Client &client) {
	// Parsing the arguments
	std::stringstream ss(args);
	std::vector<std::string> parsed_args;
	std::string arg;

	// Extract arguments to a vector
	while (ss >> arg) {
		parsed_args.push_back(arg);
	}

	// Check if the number of arguments is correct
	if (parsed_args.size() != 4) {
		printError("Wrong number of arguments");
		return;
	}

	// Check if the user is logged in
	if (client.isLoggedIn() == false) {
		printError("User not logged in.");
		return;
	}

	// Defining the arguments extracted
	std::string name = parsed_args[0];
	std::string asset_path = parsed_args[1];
	std::string start_value = parsed_args[2];
	std::string timeactive = parsed_args[3];

	// Verifying parameters
	if (verify_name(name) == -1) {
		printError("Incorrect auction name.");
		return;
	}

	if (verify_asset_fname(asset_path) == -1) {
		printError("Incorrect asset file path/name.");
		return;
	}

	if (verify_start_value(start_value) == -1) {
		printError("Incorrect start value.");
		return;
	}

	if (verify_timeactive(timeactive) == -1) {
		printError("Incorrect time active.");
		return;
	}

	// Path to file
	std::string assetf_name =
		asset_path.substr(asset_path.find_last_of("/\\") + 1);

	// Fill and send message
	ClientOpenAuction message_out;
	message_out.user_id = client.getLoggedInUser();
	message_out.password = client.getPassword();
	message_out.name = name;
	message_out.start_value = convert_auction_value(start_value);
	message_out.timeactive = static_cast<uint32_t>(stol(timeactive));
	message_out.assetf_name = assetf_name;
	message_out.Fsize = static_cast<size_t>(getFileSize(asset_path));

	int Fsize = static_cast<int>(message_out.Fsize);

	if (Fsize == -1) {
		throw FileException();
		return;
	}

	message_out.fdata = readFromFile(asset_path);

	// Message to receive
	ServerOpenAuction message_in;
	if (client.sendTcpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	};

	// Check status of the received message
	switch (message_in.status) {
		case ServerOpenAuction::status::OK:
			printOpenAuction(message_in);
			break;

		case ServerOpenAuction::status::NOK:
			printError("Couldn't start auction.");
			break;

		case ServerOpenAuction::status::NLG:
			printError("Not logged in.");
			break;

		case ServerOpenAuction::status::ERR:
			printError("Wrong format sent.");
			break;

		default:
			throw InvalidMessageException();
	}
}

/**
 * @brief Handles the close auction command.
 * @param  args: arguments following the command
 * @param  &client: reference to the client instance
 * @retval None
 */
void CloseAuctionCommand::handle(std::string args, Client &client) {
	// Parsing the arguments
	std::stringstream ss(args);
	std::vector<std::string> parsed_args;
	std::string arg;

	// Extract arguments to a vector
	while (ss >> arg) {
		parsed_args.push_back(arg);
	}

	// Check if the number of arguments is correct
	if (parsed_args.size() != 1) {
		printError("Wrong number of arguments");
		return;
	}

	// Check if the user is logged in
	if (client.isLoggedIn() == false) {
		printError("User not Logged In. Please login first.");
		return;
	}

	// Defining the arguments extracted
	std::string a_id = parsed_args[0];

	// Verifying parameters
	if (verify_auction_id(a_id) == -1) {
		printError("Incorrect AID.");
		return;
	}

	// Fill and send message
	ClientCloseAuction message_out;
	message_out.user_id = client.getLoggedInUser();
	message_out.password = client.getPassword();
	message_out.auction_id = static_cast<uint32_t>(stoi(a_id));

	// Message to receive
	ServerCloseAuction message_in;
	if (client.sendTcpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	};

	// Check status of the received message
	switch (message_in.status) {
		case ServerCloseAuction::status::OK:
			printCloseAuction(message_out);
			break;

		case ServerCloseAuction::status::NLG:
			printError("Not logged in.");
			break;

		case ServerCloseAuction::status::EAU:
			printError("Auction doesn't exist.");
			break;

		case ServerCloseAuction::status::EOW:
			printError("Auction doesn't belong to this user.");
			break;

		case ServerCloseAuction::status::END:
			printError("Auction already ended");
			break;

		case ServerCloseAuction::status::NOK:
			printError("UID doesn't exist or wrong Password.");
			break;

		case ServerCloseAuction::status::ERR:
			printError("Wrong format sent.");
			break;

		default:
			throw InvalidMessageException();
	}
}

/**
 * @brief Handles the show asset command.
 * @param  args: arguments following the command
 * @param  &client: reference to the client instance
 * @retval None
 */
void ShowAssetCommand::handle(std::string args, Client &client) {
	// Parsing the arguments
	std::stringstream ss(args);
	std::vector<std::string> parsed_args;
	std::string arg;

	// Extract arguments to a vector
	while (ss >> arg) {
		parsed_args.push_back(arg);
	}

	// Check if the number of arguments is correct
	if (parsed_args.size() != 1) {
		printError("Wrong number of arguments");
		return;
	}

	// Defining the arguments extracted
	std::string a_id = parsed_args[0];

	// Verifying parameters
	if (verify_auction_id(a_id) == -1) {
		printError("Incorrect AID.");
		return;
	}

	// Fill and send message
	ClientShowAsset message_out;
	message_out.auction_id = static_cast<uint32_t>(stoi(a_id));

	// Message to receive
	ServerShowAsset message_in;
	if (client.sendTcpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	};

	// Check status of the received message
	switch (message_in.status) {
		case ServerShowAsset::status::OK:
			printShowAsset(message_out, message_in);
			saveToFile(message_in.fname, CLIENT_ASSET_DEFAULT_PATH,
			           message_in.fdata);
			break;

		case ServerShowAsset::status::NOK:
			printError("Auction doesn't exist.");
			break;

		case ServerShowAsset::status::ERR:
			printError("Wrong format sent.");
			break;

		default:
			throw InvalidMessageException();
	}
}

/**
 * @brief Handles the bid command.
 * @param  args: arguments following the command
 * @param  &client: reference to the client instance
 * @retval None
 */
void BidCommand::handle(std::string args, Client &client) {
	// Parsing the arguments
	std::stringstream ss(args);
	std::vector<std::string> parsed_args;
	std::string arg;

	// Extract arguments to a vector
	while (ss >> arg) {
		parsed_args.push_back(arg);
	}

	// Check if the number of arguments is correct
	if (parsed_args.size() != 2) {
		printError("Wrong number of arguments");
		return;
	}

	// Check if the user is logged in
	if (client.isLoggedIn() == false) {
		printError("User not Logged In. Please login first.");
		return;
	}

	// Defining the arguments extracted
	std::string a_id = parsed_args[0];
	uint32_t value = static_cast<uint32_t>(stol(parsed_args[1]));

	// Verifying parameters
	if (verify_auction_id(a_id) == -1) {
		printError("Incorrect AID.");
		return;
	}

	if (verify_value(value) == -1) {
		printError("Incorrect value.");
		return;
	}

	// Fill and send message
	ClientBid message_out;
	message_out.user_id = client.getLoggedInUser();
	message_out.password = client.getPassword();
	message_out.auction_id = static_cast<uint32_t>(stoi(a_id));
	message_out.value = value;

	// Message to receive
	ServerBid message_in;
	if (client.sendTcpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	};

	// Check status of the received message
	switch (message_in.status) {
		case ServerBid::status::NOK:
			printError("Auction isn't active or does not exist.");
			break;

		case ServerBid::status::NLG:
			printError("User not logged in.");
			break;

		case ServerBid::status::ACC:
			printBid(message_out);
			break;

		case ServerBid::status::REF:
			printError("A larger bid was already placed.");
			break;

		case ServerBid::status::ILG:
			printError("A auction host user can't bid on his own auction.");
			break;

		case ServerBid::status::ERR:
			printError("Wrong format sent.");
			break;

		default:
			throw InvalidMessageException();
	}
}

/**
 * @brief Handles the exit command.
 * @param  args: arguments following the command
 * @param  &client: reference to the client instance
 * @retval None
 */
void ExitCommand::handle(std::string args, Client &client) {
	(void) args;  // unused - no args

	// Verify if the client is logged in
	if (client.isLoggedIn() == false) {
		// Logout and exit
		printSuccess("Shutting down.");
		client.~Client();
		exit(EXIT_SUCCESS);
	}

	// Fill and send message
	ClientLogout message_out;
	message_out.user_id = client.getLoggedInUser();
	message_out.password = client.getPassword();

	// Message to receive
	ServerLogout message_in;
	if (client.sendUdpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	}

	// Check status of the received message
	switch (message_in.status) {
		case ServerLogout::status::OK:
			client.logout();
			printSuccess("Sucessfully logged out");
			break;

		case ServerLogout::status::NOK:
			printError("Couldn't logout.");
			break;

		case ServerLogout::status::UNR:
			printError("Unregistered user.");
			break;

		case ServerLogout::status::ERR:
			printError("Wrong format sent.");
			break;

		default:
			throw InvalidMessageException();
	}

	// Exit
	printSuccess("Shutting down.");
	client.~Client();
	exit(EXIT_SUCCESS);
}

/**
 * @brief Handles the help command.
 * @param  args: arguments following the command
 * @param  &client: reference to the client instance
 * @retval None
 */
void HelpCommand::handle(std::string args, Client &client) {
	(void) args;    // unused - no args
	(void) client;  // unused client

	// Call the command manager to print the help message
	_manager.printHelp();
}

/**
 * @brief Register a set of commands on the command manager
 * @param  &manager: reference to the command manager instance
 * @retval None
 */
void registerCommands(CommandManager &manager) {
	manager.registerCommand(std::make_shared<LoginCommand>());
	manager.registerCommand(std::make_shared<OpenAuctionCommand>());
	manager.registerCommand(std::make_shared<CloseAuctionCommand>());
	manager.registerCommand(std::make_shared<ListStartedAuctionsCommand>());
	manager.registerCommand(std::make_shared<ListBiddedAuctionsCommand>());
	manager.registerCommand(std::make_shared<ListAllAuctionsCommand>());
	manager.registerCommand(std::make_shared<ShowAssetCommand>());
	manager.registerCommand(std::make_shared<BidCommand>());
	manager.registerCommand(std::make_shared<ShowRecordCommand>());
	manager.registerCommand(std::make_shared<LogoutCommand>());
	manager.registerCommand(std::make_shared<UnregisterCommand>());
	manager.registerCommand(std::make_shared<ExitCommand>());
	manager.registerCommand(std::make_shared<HelpCommand>(manager));
}
