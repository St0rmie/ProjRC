#include "commands.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include "client.hpp"
#include "shared/protocol.hpp"
#include "shared/verifications.hpp"

void CommandManager::registerCommand(std::shared_ptr<CommandHandler> handler) {
	this->handlers.insert({handler->_name, handler});
	if (handler->_alias) {
		this->handlers.insert({*handler->_alias, handler});
	}
}

void CommandManager::waitCommand(Client &client) {
	std::cout << "> ";

	std::string line;
	std::getline(std::cin, line);

	if (std::cin.eof()) {
		return;
	}

	int splitIndex = line.find(' ');

	std::string commandName;
	if (splitIndex == line.length() - 1) {
		commandName = line;
		line = "";
	} else {
		commandName = line.substr(0, splitIndex);
		line = line.substr(splitIndex + 1, line.length() - 1);
	}

	if (commandName.length() == 0) {
		return;
	}

	auto handler = this->handlers.find(commandName);
	if (handler == this->handlers.end()) {
		std::cout << "[ERROR] Unknown Command. Type \"help\" for the list of "
					 "commands available"
				  << std::endl;
		return;
	}

	handler->second->handle(line, client);
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
		std::cout << "[ERROR] Wrong number of arguments" << std::endl;
		return;
	}

	if (client.isLoggedIn() == true) {
		std::cout << "[ERROR] Already Logged In. Please logout first."
				  << std::endl;
		return;
	}

	// Defining the arguments extracted
	std::string user_id = parsed_args[0];
	std::string password = parsed_args[1];

	// Verifying parameters
	if (verify_user_id(user_id) == -1) {
		std::cout << "[ERROR] Incorrect user id." << std::endl;
		return;
	}
	if (verify_password(password) == -1) {
		std::cout << "[ERROR] Incorrect password." << std::endl;
		return;
	}

	// Populate and send packet
	ClientLoginUser message_out;
	message_out.user_id = convert_user_id(user_id);
	message_out.password = convert_password(password);

	ServerLoginUser message_in;
	client.sendUdpMessageAndAwaitReply(message_out, message_in);

	// Check status
	switch (message_in.status) {
		case ServerLoginUser::status::OK:
			client.login(convert_user_id(user_id));
			std::cout << "[Login] Sucessfully logged in as "
					  << client.getLoggedInUser() << std::endl;
			break;

		case ServerLoginUser::status::NOK:
			std::cout << "[Login] Couldn't login." << std::endl;
			break;

		case ServerLoginUser::status::REG:
			client.login(convert_user_id(user_id));
			std::cout << "[Login] Registered user." << std::endl;
			break;

		default:
			throw InvalidMessageException();
	}
}

void CreateAuctionCommand::handle(std::string args, Client &client) {
	// Parsing the arguments
	std::stringstream ss(args);
	std::vector<std::string> parsed_args;
	std::string arg;

	while (ss >> arg) {
		parsed_args.push_back(arg);
	}

	if (parsed_args.size() != 4) {
		std::cout << "[ERROR] Wrong number of arguments" << std::endl;
		return;
	}

	// Defining the arguments extracted
	std::string name = parsed_args[0];
	std::string asset_fname = parsed_args[1];
	std::string start_value = parsed_args[2];
	std::string timeactive = parsed_args[3];

	// Verifying parameters
	if (verify_name(name) == -1) {
		std::cout << "[ERROR] Incorrect auction name." << std::endl;
		return;
	}
	if (verify_asset_fname(asset_fname) == -1) {
		std::cout << "[ERROR] Incorrect asset file name." << std::endl;
		return;
	}

	if (verify_start_value(start_value) == -1) {
		std::cout << "[ERROR] Incorrect start value." << std::endl;
		return;
	}
	if (verify_timeactive(timeactive) == -1) {
		std::cout << "[ERROR] Incorrect time active." << std::endl;
		return;
	}

	// Protocol setup
	std::cout << "CREATED AUCTION // AUCTION: " << name
			  << " // FILE: " << asset_fname
			  << " // START VALUE: " << start_value
			  << " // TIME ACTIVE: " << timeactive << std::endl;
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
		std::cout << "[ERROR] Wrong number of arguments" << std::endl;
		return;
	}

	std::string a_id = parsed_args[0];

	if (verify_auction_id(a_id) == -1) {
		std::cout << "[ERROR] Incorrect AID." << std::endl;
		return;
	}

	// Protocol setup
	std::cout << "CLOSED AUCTION // AUCTION: " << a_id << std::endl;
}

void ListStartedAuctionsCommand::handle(std::string args, Client &client) {
	(void) args;  // unused - no args

	// Protocol setup
	std::cout << "LISTED STARTED AUCTIONS" << std::endl;
}

void ListBiddedAuctionsCommand::handle(std::string args, Client &client) {
	(void) args;  // unused - no args

	// Protocol setup
	std::cout << "LISTED BIDDED AUCTIONS" << std::endl;
}

void ListAllAuctionsCommand::handle(std::string args, Client &client) {
	(void) args;  // unused - no args

	// Protocol setup
	std::cout << "LISTED ALL AUCTIONS" << std::endl;
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
		std::cout << "[ERROR] Wrong number of arguments" << std::endl;
		return;
	}

	std::string a_id = parsed_args[0];

	if (verify_auction_id(a_id) == -1) {
		std::cout << "[ERROR] Incorrect AID." << std::endl;
		return;
	}

	// Protocol setup
	std::cout << "SHOWED IMAGE FILE OF THE ASSET: " << a_id << std::endl;
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
		std::cout << "[ERROR] Wrong number of arguments" << std::endl;
		return;
	}

	std::string a_id = parsed_args[0];
	uint32_t value = stoi(parsed_args[1]);

	if (verify_auction_id(a_id) == -1) {
		std::cout << "[ERROR] Incorrect AID." << std::endl;
		return;
	}

	if (verify_value(value) == -1) {
		std::cout << "[ERROR] Incorrect value." << std::endl;
		return;
	}

	// Protocol setup
	std::cout << "BIDDED ON // AUCTION: " << a_id << " // VALUE: " << value
			  << std::endl;
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
		std::cout << "[ERROR] Wrong number of arguments" << std::endl;
		return;
	}

	std::string a_id = parsed_args[0];

	if (verify_auction_id(a_id) == -1) {
		std::cout << "[ERROR] Incorrect AID." << std::endl;
		return;
	}

	// Protocol setup
	std::cout << "SHOWED RECORD // AUCTION: " << a_id << std::endl;
}

void LogoutCommand::handle(std::string args, Client &client) {
	(void) args;  // unused - no args

	// Protocol setup
	std::cout << "LOGGED OUT" << std::endl;
}

void UnregisterCommand::handle(std::string args, Client &client) {
	(void) args;  // unused - no args

	// Protocol setup
	std::cout << "UNREGISTERED" << std::endl;
}

void ExitCommand::handle(std::string args, Client &client) {
	(void) args;  // unused - no args

	// Protocol setup
	std::cout << "EXITED" << std::endl;
}