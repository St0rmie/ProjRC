#include "output.hpp"

void printRecord(std::string aid, ServerShowRecord message) {
	std::string active_status = message.end_sec_time <= 0 ? "active" : "closed";
	std::cout << "[SUCCESS] Showing record for auction " << aid
			  << ":\n=================================================="
			  << "\n\tHost ID:\t\t" << message.host_UID
			  << "\n\tAuction Name:\t\t" << message.auction_name
			  << "\n\tAsset File Name:\t" << message.asset_fname
			  << "\n\tStart Value:\t\t" << message.start_value
			  << "\n\tStart Date:\t\t" << extractDate(message.start_date_time)
			  << "\n\tStart Time:\t\t" << extractTime(message.start_date_time)
			  << "\n\tTimeactive:\t\t" << message.timeactive
			  << "\n\tStatus:\t\t\t" << active_status << "\n";
	if (message.bids.size() > 0) {
		std::cout << "==================================================\n"
				  << "\t\tBIDS (" << message.bids.size() << ")" << std::endl;
		std::cout << "--------------------------------------------------"
				  << "\nBidderUID\tBid Value\tBid Date\tBid Time\tTime "
					 "Elapsed\n"
				  << "--------------------------------------------------"
				  << std::endl;
	}
	for (Bid bid : message.bids) {
		std::cout << bid.bidder_UID << "\t\t" << bid.bid_value << "\t\t"
				  << extractDate(bid.bid_date_time) << "\t"
				  << extractTime(bid.bid_date_time) << "\t" << bid.bid_sec_time
				  << "\n";
	}
	if (message.end_sec_time > 0) {
		std::cout << "==================================================\n"
				  << "END\tEnd Date\tEnd Time\tEnd Elapsed\n"
				  << "--------------------------------------------------\n";
		std::cout << "OVER\t" << extractDate(message.end_date_time) << "\t"
				  << extractTime(message.end_date_time) << "\t"
				  << message.end_sec_time << std::endl;
	}
	std::cout << "==================================================\n"
			  << "END RECORD\n";
}

void printOpenAuction(ServerOpenAuction message) {
	std::cout << "[SUCCESS] Auction " << message.auction_id << " opened."
			  << std::endl;
}

void printCloseAuction(ClientCloseAuction message) {
	std::cout << "[SUCCESS] Auction " << message.auction_id << " closed."
			  << std::endl;
}

void printShowAsset(ClientShowAsset message_c, ServerShowAsset message_s) {
	std::cout << "[SUCCESS] Retrieved " << message_c.auction_id
			  << " auction's asset.\n\tFile Name: " << message_s.fname
			  << std::setprecision(3) << std::fixed
			  << "\n\tFile Size: " << (float) message_s.fsize / (1000000)
			  << " MB" << std::endl;
}

void printBid(ClientBid message) {
	std::cout << "[SUCCESS] Bidded " << message.value << " on auction "
			  << message.auction_id << "." << std::endl;
}

void printError(std::string str) {
	std::cout << "[ERROR] " << str << std::endl;
}

void printSuccess(std::string success_description) {
	std::cout << "[SUCCESS] " << success_description << std::endl;
}