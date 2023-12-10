#include "client_main.hpp"

#include <iostream>
#include <memory>
#include <string>

#include "client.hpp"
#include "commands.hpp"

int main(int argc, char *argv[]) {
	Client client(argc, argv);
	CommandManager commandManager;
	registerCommands(commandManager);

	while (!std::cin.eof()) {
		try {
			commandManager.waitCommand(client);
		} catch (MessageReceiveException &e) {
			printError("Server didn't answer.");
		} catch (MessageSendException &e) {
			printError("Failed to send message.");
		} catch (MessageBuildingException &e) {
			printError("Failed to build message.");
		} catch (InvalidMessageException &e) {
			printError("Invalid message received (Wrong format).");
		} catch (UnexpectedMessageException &e) {
			printError("Invalid message (ERR).");
		} catch (std::exception &e) {
			std::cout << e.what() << std::endl;
		}
	}

	std::cout << "[QUIT] Shutting Down." << std::endl;
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
}