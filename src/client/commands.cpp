#include "commands.hpp"

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
	std::cout << "> ";

	std::string line;
	std::getline(std::cin, line);

	if (std::cin.eof()) {
		return;
	}

	int splitIndex = line.find(' ');

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

		default:
			throw InvalidMessageException();
	}
}

void LogoutCommand::handle(std::string args, Client &client) {
	if (args.length() > 0) {
		std::cout << "[ERROR] Wrong number of arguments" << std::endl;
		return;
	}

	if (client.isLoggedIn() == false) {
		std::cout << "[ERROR] Not logged in. Please login first." << std::endl;
		return;
	}

	// Populate and send packet
	ClientLogout message_out;
	message_out.user_id = client.getLoggedInUser();
	message_out.password = client.getPassword();

	ServerLogout message_in;
	client.sendUdpMessageAndAwaitReply(message_out, message_in);

	// Check status
	switch (message_in.status) {
		case ServerLogout::status::OK:
			client.logout();
			std::cout << "[SUCCESS] Sucessfully logged out" << std::endl;
			break;

		case ServerLogout::status::NOK:
			std::cout << "[ERROR] Couldn't logout." << std::endl;
			break;

		case ServerLogout::status::UNR:
			std::cout << "[ERROR] Unregistered user." << std::endl;
			break;

		default:
			throw InvalidMessageException();
	}
}

void UnregisterCommand::handle(std::string args, Client &client) {
	if (args.length() > 0) {
		std::cout << "[ERROR] Wrong number of arguments" << std::endl;
		return;
	}

	if (client.isLoggedIn() == false) {
		std::cout << "[ERROR] Not logged in. Please login first." << std::endl;
		return;
	}

	// Populate and send packet
	ClientUnregister message_out;
	message_out.user_id = client.getLoggedInUser();
	message_out.password = client.getPassword();

	ServerUnregister message_in;
	client.sendUdpMessageAndAwaitReply(message_out, message_in);

	// Check status
	switch (message_in.status) {
		case ServerUnregister::status::OK:
			client.logout();
			std::cout << "[SUCCESS] Sucessfully unregister." << std::endl;
			break;

		case ServerUnregister::status::NOK:
			std::cout << "[ERROR] Not logged in, hence couldn't unregister."
					  << std::endl;
			break;

		case ServerUnregister::status::UNR:
			std::cout << "[ERROR] Unregistered user." << std::endl;
			break;

		default:
			throw InvalidMessageException();
	}
}

void ListStartedAuctionsCommand::handle(std::string args, Client &client) {
	if (args.length() > 0) {
		std::cout << "[ERROR] Wrong number of arguments" << std::endl;
		return;
	}

	if (client.isLoggedIn() == false) {
		std::cout << "[ERROR] Not logged in. Please login first." << std::endl;
		return;
	}

	// Populate and send packet
	ClientListStartedAuctions message_out;
	message_out.user_id = client.getLoggedInUser();

	ServerListStartedAuctions message_in;
	client.sendUdpMessageAndAwaitReply(message_out, message_in);

	// Check status
	switch (message_in.status) {
		case ServerListStartedAuctions::status::OK:;
			std::cout << "[SUCCESS] Listing \nAuctions started by user "
					  << client.getLoggedInUser() << ":" << std::endl;
			for (std::string auc : message_in.auctions) {
				std::cout << "\t" << auc << std::endl;
			}
			break;

		case ServerListStartedAuctions::status::NOK:
			std::cout << "[ERROR] User doesn't have ongoing auctions."
					  << std::endl;
			break;

		case ServerListStartedAuctions::status::NLG:
			std::cout << "[ERROR] User not logged in." << std::endl;
			break;

		default:
			throw InvalidMessageException();
	}
}

void ListBiddedAuctionsCommand::handle(std::string args, Client &client) {
	if (args.length() > 0) {
		std::cout << "[ERROR] Wrong number of arguments" << std::endl;
		return;
	}

	if (client.isLoggedIn() == false) {
		std::cout << "[ERROR] Not logged in. Please login first." << std::endl;
		return;
	}

	// Populate and send packet
	ClientListBiddedAuctions message_out;
	message_out.user_id = client.getLoggedInUser();

	ServerListBiddedAuctions message_in;
	client.sendUdpMessageAndAwaitReply(message_out, message_in);

	// Check status
	switch (message_in.status) {
		case ServerListStartedAuctions::status::OK:;
			std::cout << "[SUCCESS] Listing \nAuctions bidded by user "
					  << client.getLoggedInUser() << ":" << std::endl;
			for (std::string auc : message_in.auctions) {
				std::cout << "\t" << auc << std::endl;
			}
			break;

		case ServerListStartedAuctions::status::NOK:
			std::cout << "[ERROR] User didn't bid on any auctions."
					  << std::endl;
			break;

		case ServerListStartedAuctions::status::NLG:
			std::cout << "[ERROR] User not logged in." << std::endl;
			break;

		default:
			throw InvalidMessageException();
	}
}

void ListAllAuctionsCommand::handle(std::string args, Client &client) {
	if (args.length() > 0) {
		std::cout << "[ERROR] Wrong number of arguments" << std::endl;
		return;
	}

	if (client.isLoggedIn() == false) {
		std::cout << "[ERROR] Not logged in. Please login first." << std::endl;
		return;
	}

	// Populate and send packet
	ClientListAllAuctions message_out;
	message_out.user_id = client.getLoggedInUser();

	ServerListAllAuctions message_in;
	client.sendUdpMessageAndAwaitReply(message_out, message_in);

	// Check status
	switch (message_in.status) {
		case ServerListAllAuctions::status::OK:;
			std::cout
				<< "[SUCCESS] Listing \nAuctions registered on the server:"
				<< ":" << std::endl;
			for (std::string auc : message_in.auctions) {
				std::cout << "\t" << auc << std::endl;
			}
			break;

		case ServerListAllAuctions::status::NOK:
			std::cout << "[ERROR] There aren't any auctions on the system yet."
					  << std::endl;
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
		std::cout << "[ERROR] Wrong number of arguments" << std::endl;
		return;
	}

	std::string a_id = parsed_args[0];

	if (verify_auction_id(a_id) == -1) {
		std::cout << "[ERROR] Incorrect AID." << std::endl;
		return;
	}

	// Populate and send packet
	ClientShowRecord message_out;
	message_out.auction_id = convert_auction_id(a_id);

	ServerShowRecord message_in;
	client.sendUdpMessageAndAwaitReply(message_out, message_in);

	// Check status
	switch (message_in.status) {
		case ServerShowRecord::status::OK:;
			printRecord(a_id, message_in);
			break;

		case ServerShowRecord::status::NOK:
			std::cout << "[ERROR] There aren't any auctions on the system yet."
					  << std::endl;
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

	// Path to file
	std::string pathname = "assets/" + asset_fname;

	// Protocol setup
	// Populate and send message
	ClientOpenAuction message_out;
	message_out.user_id = client.getLoggedInUser();
	message_out.password = client.getPassword();
	message_out.name = name;
	message_out.start_value = convert_auction_value(start_value);
	message_out.timeactive = stoi(timeactive);
	message_out.assetf_name = asset_fname;
	message_out.fsize = getFileSize(pathname);
	if (message_out.fsize == -1) {
		throw FileException();
		return;
	}
	message_out.fdata = readFromFile(pathname, message_out.fsize);

	ServerOpenAuction message_in;
	client.sendTcpMessageAndAwaitReply(message_out, message_in);

	// Check status
	switch (message_in.status) {
		case ServerOpenAuction::status::OK:
			printOpenAuction(message_in);
			break;

		case ServerOpenAuction::status::NOK:
			std::cout << "[ERROR] Couldn't start auction." << std::endl;
			break;

		case ServerOpenAuction::status::NLG:
			std::cout << "[ERROR] Not logged in." << std::endl;

		case ServerOpenAuction::status::ERR:
			std::cout << "[ERROR] Wrong result." << std::endl;

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
		std::cout << "[ERROR] Wrong number of arguments" << std::endl;
		return;
	}

	std::string a_id = parsed_args[0];

	if (verify_auction_id(a_id) == -1) {
		std::cout << "[ERROR] Incorrect AID." << std::endl;
		return;
	}

	// Protocol setup
	// Populate and send message
	ClientCloseAuction message_out;
	message_out.user_id = client.getLoggedInUser();
	message_out.password = client.getPassword();
	message_out.auction_id = stoi(a_id);

	ServerCloseAuction message_in;
	client.sendTcpMessageAndAwaitReply(message_out, message_in);

	// Check status
	switch (message_in.status) {
		case ServerCloseAuction::status::OK:
			printCloseAuction(message_out);
			break;

		case ServerCloseAuction::status::NLG:
			std::cout << "[ERROR] Not logged in." << std::endl;
			break;

		case ServerCloseAuction::status::EAU:
			std::cout << "[ERROR] Auction doesn't exist." << std::endl;
			break;

		case ServerCloseAuction::status::EOW:
			std::cout << "[ERROR] Auction doesn't belong to this user."
					  << std::endl;
			break;

		case ServerCloseAuction::status::END:
			std::cout << "[ERROR] Auction already ended" << std::endl;
			break;

		case ServerCloseAuction::status::ERR:
			std::cout << "[ERROR] Wrong result." << std::endl;
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

void ExitCommand::handle(std::string args, Client &client) {
	(void) args;  // unused - no args

	// Protocol setup
	std::cout << "EXITED" << std::endl;
}