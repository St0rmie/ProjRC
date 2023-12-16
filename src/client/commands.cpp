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

void CommandManager::registerCommand(std::shared_ptr<CommandHandler> handler) {
	this->handlers.insert({handler->_name, handler});
	if (handler->_alias) {
		this->handlers.insert({*handler->_alias, handler});
	}
}

void CommandManager::waitCommand(Client &client) {
	std::string line_base;
	if (client.isLoggedIn()) {
		line_base += "[";
		line_base += std::to_string(client.getLoggedInUser());
		line_base += "] > ";
	} else {
		line_base += "> ";
	}

	char *input = readline(line_base.c_str());
	add_history(input);
	std::string line(input);
	free(input);

	size_t splitIndex = line.find(' ');

	std::string commandName;
	if (splitIndex == std::string::npos) {
		commandName = line;
		line = "";
	} else {
		commandName = line.substr(0, splitIndex);
		line.erase(0, splitIndex + 1);
	}

	if (commandName.length() == 0) {
		return;
	}

	auto handler = this->handlers.find(commandName);
	if (handler == this->handlers.end()) {
		printError(
			"Unknown Command. Type \"help\" for the list of "
			"commands available");
		return;
	}

	handler->second->handle(line, client);
}

void CommandManager::printHelp() {
	std::cout << "[HELP] Available commands:" << std::endl << std::left;

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

void LoginCommand::handle(std::string args, Client &client) {
	// Parsing the arguments
	std::stringstream ss(args);
	std::vector<std::string> parsed_args;
	std::string arg;

	while (ss >> arg) {
		parsed_args.push_back(arg);
	}

	if (parsed_args.size() != 2) {
		printError("Wrong number of arguments");
		return;
	}

	if (client.isLoggedIn() == true) {
		printError("[Already Logged In. Please logout first.");
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

	// Populate and send packet
	ClientLoginUser message_out;
	message_out.user_id = convert_user_id(user_id);
	message_out.password = convert_password(password);

	ServerLoginUser message_in;
	if (client.sendUdpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	}

	// Check status
	switch (message_in.status) {
		case ServerLoginUser::status::OK:
			client.login(convert_user_id(user_id), convert_password(password));
			std::cout << "[Login] Sucessfully logged in as "
					  << client.getLoggedInUser() << std::endl;
			break;

		case ServerLoginUser::status::NOK:
			std::cout << "[Login] Couldn't login." << std::endl;
			break;

		case ServerLoginUser::status::REG:
			client.login(convert_user_id(user_id), convert_password(password));
			std::cout << "[Login] Registered user." << std::endl;
			break;

		case ServerLoginUser::status::ERR:
			printError("Wrong format sent.");
			break;

		default:
			throw InvalidMessageException();
	}
}

void LogoutCommand::handle(std::string args, Client &client) {
	if (args.length() > 0) {
		printError("Wrong number of arguments");
		return;
	}

	if (client.isLoggedIn() == false) {
		printError("Not logged in. Please login first.");
		return;
	}

	// Populate and send packet
	ClientLogout message_out;
	message_out.user_id = client.getLoggedInUser();
	message_out.password = client.getPassword();

	ServerLogout message_in;
	if (client.sendUdpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	}

	// Check status
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

void UnregisterCommand::handle(std::string args, Client &client) {
	if (args.length() > 0) {
		printError("Wrong number of arguments");
		return;
	}

	if (client.isLoggedIn() == false) {
		printError("Not logged in. Please login first.");
		return;
	}

	// Populate and send packet
	ClientUnregister message_out;
	message_out.user_id = client.getLoggedInUser();
	message_out.password = client.getPassword();

	ServerUnregister message_in;
	if (client.sendUdpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	}

	// Check status
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

void ListStartedAuctionsCommand::handle(std::string args, Client &client) {
	if (args.length() > 0) {
		printError("Wrong number of arguments");
		return;
	}

	if (client.isLoggedIn() == false) {
		printError("Not logged in. Please login first.");
		return;
	}

	// Populate and send packet
	ClientListStartedAuctions message_out;
	message_out.user_id = client.getLoggedInUser();

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

	ServerListBiddedAuctions message_in;
	if (client.sendUdpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	}

	std::string message_ok =
		"Listing \nAuctions started by user " + client.getLoggedInUser();
	message_ok += ":";

	// Check status
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

void ListAllAuctionsCommand::handle(std::string args, Client &client) {
	if (args.length() > 0) {
		printError("Wrong number of arguments");
		return;
	}

	// Populate and send packet
	ClientListAllAuctions message_out;

	ServerListAllAuctions message_in;
	if (client.sendUdpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	}

	// Check status
	switch (message_in.status) {
		case ServerListAllAuctions::status::OK:;
			printSuccess("Listing \nAuctions registered on the server:");
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

void ShowRecordCommand::handle(std::string args, Client &client) {
	// Parsing the arguments
	std::stringstream ss(args);
	std::vector<std::string> parsed_args;
	std::string arg;

	while (ss >> arg) {
		parsed_args.push_back(arg);
	}

	if (parsed_args.size() != 1) {
		printError("Wrong number of arguments");
		return;
	}

	std::string a_id = parsed_args[0];

	if (verify_auction_id(a_id) == -1) {
		printError("Incorrect AID.");
		return;
	}

	// Populate and send packet
	ClientShowRecord message_out;
	message_out.auction_id = convert_auction_id(a_id);

	ServerShowRecord message_in;
	if (client.sendUdpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	}

	// Check status
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

void OpenAuctionCommand::handle(std::string args, Client &client) {
	// Parsing the arguments
	std::stringstream ss(args);
	std::vector<std::string> parsed_args;
	std::string arg;

	while (ss >> arg) {
		parsed_args.push_back(arg);
	}

	if (parsed_args.size() != 4) {
		printError("Wrong number of arguments");
		return;
	}

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
	/*if (verify_name(name) == -1) {
	    printError("Incorrect auction name.");
	    return;
	}*/
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

	// Protocol setup
	// Populate and send message
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

	ServerOpenAuction message_in;
	if (client.sendTcpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	};

	// Check status
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

void CloseAuctionCommand::handle(std::string args, Client &client) {
	// Parsing the arguments
	std::stringstream ss(args);
	std::vector<std::string> parsed_args;
	std::string arg;

	while (ss >> arg) {
		parsed_args.push_back(arg);
	}

	if (parsed_args.size() != 1) {
		printError("Wrong number of arguments");
		return;
	}

	if (client.isLoggedIn() == false) {
		printError("User not Logged In. Please login first.");
		return;
	}

	std::string a_id = parsed_args[0];

	if (verify_auction_id(a_id) == -1) {
		printError("Incorrect AID.");
		return;
	}

	// Protocol setup
	// Populate and send message
	ClientCloseAuction message_out;
	message_out.user_id = client.getLoggedInUser();
	message_out.password = client.getPassword();
	message_out.auction_id = static_cast<uint32_t>(stoi(a_id));

	ServerCloseAuction message_in;
	if (client.sendTcpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	};

	// Check status
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

void ShowAssetCommand::handle(std::string args, Client &client) {
	// Parsing the arguments
	std::stringstream ss(args);
	std::vector<std::string> parsed_args;
	std::string arg;

	while (ss >> arg) {
		parsed_args.push_back(arg);
	}

	if (parsed_args.size() != 1) {
		printError("Wrong number of arguments");
		return;
	}

	std::string a_id = parsed_args[0];

	if (verify_auction_id(a_id) == -1) {
		printError("Incorrect AID.");
		return;
	}

	// Protocol setup
	// Populate and send message
	ClientShowAsset message_out;
	message_out.auction_id = static_cast<uint32_t>(stoi(a_id));

	ServerShowAsset message_in;
	if (client.sendTcpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	};

	// Check status
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

void BidCommand::handle(std::string args, Client &client) {
	// Parsing the arguments
	std::stringstream ss(args);
	std::vector<std::string> parsed_args;
	std::string arg;

	while (ss >> arg) {
		parsed_args.push_back(arg);
	}

	if (parsed_args.size() != 2) {
		printError("Wrong number of arguments");
		return;
	}

	if (client.isLoggedIn() == false) {
		printError("User not Logged In. Please login first.");
		return;
	}

	std::string a_id = parsed_args[0];
	uint32_t value = static_cast<uint32_t>(stol(parsed_args[1]));

	if (verify_auction_id(a_id) == -1) {
		printError("Incorrect AID.");
		return;
	}

	if (verify_value(value) == -1) {
		printError("Incorrect value.");
		return;
	}

	// Protocol setup
	// Populate and send message
	ClientBid message_out;
	message_out.user_id = client.getLoggedInUser();
	message_out.password = client.getPassword();
	message_out.auction_id = static_cast<uint32_t>(stoi(a_id));
	message_out.value = value;

	ServerBid message_in;
	if (client.sendTcpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	};

	// Check status
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

void ExitCommand::handle(std::string args, Client &client) {
	(void) args;  // unused - no args

	if (client.isLoggedIn() == false) {
		printSuccess("Shutting down.");
		client.~Client();
		exit(EXIT_SUCCESS);
	}

	// Populate and send packet
	ClientLogout message_out;
	message_out.user_id = client.getLoggedInUser();
	message_out.password = client.getPassword();

	ServerLogout message_in;
	if (client.sendUdpMessageAndAwaitReply(message_out, message_in) == -1) {
		return;
	}

	// Check status
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

	// Protocol setup
	printSuccess("Shutting down.");
	client.~Client();
	exit(EXIT_SUCCESS);
}

void HelpCommand::handle(std::string args, Client &client) {
	(void) args;    // unused - no args
	(void) client;  // unused client

	_manager.printHelp();
}

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
