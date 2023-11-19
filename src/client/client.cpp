#include "client.hpp"

#include <iostream>
#include <memory>
#include <string>

#include "commands.hpp"

int main(int argc, char *argv[]) {
	CommandManager commandManager;
	registerCommands(commandManager);

	while (!std::cin.eof()) {
		commandManager.waitCommand();
	}

	std::cout << "Bye Bye!" << std::endl;
}

void registerCommands(CommandManager &manager) {
	manager.registerCommand(std::make_shared<LoginCommand>());
	manager.registerCommand(std::make_shared<CreateAuctionCommand>());
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
}