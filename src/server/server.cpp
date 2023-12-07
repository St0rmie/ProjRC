#include "server.hpp"
#include "handlers.hpp"

void registerRequestHandlers(RequestManager &manager) {
	manager.registerRequest(std::make_shared<LoginRequest>());
	manager.registerRequest(std::make_shared<OpenAuctionRequest>());
	manager.registerRequest(std::make_shared<CloseAuctionRequest>());
	manager.registerRequest(std::make_shared<ListStartedAuctionsRequest>());
	manager.registerRequest(std::make_shared<ListBiddedAuctionsRequest>());
	manager.registerRequest(std::make_shared<ListAllAuctionsRequest>());
	manager.registerRequest(std::make_shared<ShowAssetRequest>());
	manager.registerRequest(std::make_shared<BidRequest>());
	manager.registerRequest(std::make_shared<ShowRecordRequest>());
	manager.registerRequest(std::make_shared<LogoutRequest>());
	manager.registerRequest(std::make_shared<UnregisterRequest>());
	manager.registerRequest(std::make_shared<ExitRequest>());
}

int main(int argc, char *argv[]) {
    Server server(argc, argv);
    RequestManager requestManager;
    requestManager.registerRequestHandlers(requestManager);

	while (!std::cin.eof()) {
		requestManager.waitRequest(server);
	}
	std::cout << "[QUIT] Shutting Down." << std::endl;

  return EXIT_SUCCESS;
}