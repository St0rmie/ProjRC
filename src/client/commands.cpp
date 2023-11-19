#include "commands.hpp"
#include <iostream>
#include <string>
#include <sstream>

void CommandManager::registerCommand(std::shared_ptr<CommandHandler> handler) {
  this->handlers.insert({handler->_name, handler});
  if (handler->_alias) {
    this->handlers.insert({*handler->_alias, handler});
  }
}

void CommandManager::waitCommand() {
  std::cout << "> ";

  std::string line;
  std::getline(std::cin, line);
  
  if(std::cin.eof()){
    return;
  }

  int splitIndex = line.find(' ');

  std::string commandName;
  if(splitIndex == line.length()-1){
    commandName = line;
    line = "";
  } else {
    commandName = line.substr(0,splitIndex);
    line = line.substr(splitIndex+1,line.length()-1);
  }

  if(commandName.length() == 0){
    return;
  }

  auto handler = this->handlers.find(commandName);
  if (handler == this->handlers.end()){
    std::cout << "[ERROR] Unknown Command. Type \"help\" for the list of commands available" << std::endl;
    return;
  }

  handler->second->handle(line);
}

void LoginCommand::handle(std::string args){
  
  // Parsing the arguments
  std::stringstream ss(args);
  std::vector<std::string> parsed_args;
  std::string arg;

  while(ss >> arg){
    parsed_args.push_back(arg);
  }

  if(parsed_args.size() != 2){
    std::cout << "[ERROR] Wrong number of arguments" << std::endl;
    return;
  }

  // Defining the arguments extracted
  uint32_t user_id = stoi(parsed_args[0]);
  std::string password = parsed_args[1];

  // Verifying parameters
  if(user_id > 999999 || user_id < 0){
    std::cout << "[ERROR] Incorrect user id." << std::endl;
    return;
  }
  if(password.size() > 8){
    std::cout << "[ERROR] Incorrect password." << std::endl;
    return;
  }

  // Protocol setup
  std::cout << "LOGGED IN // USER: " << user_id << " // PASSWORD: " << password << std::endl;
}

void CreateAuctionCommand::handle(std::string args){

  // Parsing the arguments
  std::stringstream ss(args);
  std::vector<std::string> parsed_args;
  std::string arg;

  while(ss >> arg){
    parsed_args.push_back(arg);
  }

  if(parsed_args.size() != 4){
    std::cout << "[ERROR] Wrong number of arguments" << std::endl;
    return;
  }

  // Defining the arguments extracted
  std::string name = parsed_args[0];
  std::string asset_fname = parsed_args[1];
  uint32_t start_value = stoi(parsed_args[2]);
  uint32_t timeactive = stoi(parsed_args[3]);

  // Verifying parameters
  if(name.size() > 10){
    std::cout << "[ERROR] Incorrect auction name." << std::endl;
    return;
  }
  if(start_value > 999999 || start_value < 0){
    std::cout << "[ERROR] Incorrect start value." << std::endl;
    return;
  }
  if(timeactive > 99999 || timeactive < 0){
    std::cout << "[ERROR] Incorrect time active." << std::endl;
    return;
  }

  // Protocol setup
  std::cout << "CREATED AUCTION // AUCTION: " << name << " // FILE: " << asset_fname << " // START VALUE: " << start_value << " // TIME ACTIVE: " << timeactive << std::endl;
}

void CloseAuctionCommand::handle(std::string args){

  // Parsing the arguments
  std::stringstream ss(args);
  std::vector<std::string> parsed_args;
  std::string arg;

  while(ss >> arg){
    parsed_args.push_back(arg);
  }

  if(parsed_args.size() != 1){
    std::cout << "[ERROR] Wrong number of arguments" << std::endl;
    return;
  }

  uint32_t a_id = stoi(parsed_args[0]);

  if(a_id > 999 || a_id < 0){
    std::cout << "[ERROR] Incorrect AID." << std::endl;
    return;
  }

  // Protocol setup
  std::cout << "CLOSED AUCTION // AUCTION: " << a_id << std::endl;
}

void ListStartedAuctionsCommand::handle(std::string args){

  (void)args;  // unused - no args

  // Protocol setup
  std::cout << "LISTED STARTED AUCTIONS" << std::endl;

}

void ListBiddedAuctionsCommand::handle(std::string args){

  (void)args;  // unused - no args

  // Protocol setup
  std::cout << "LISTED BIDDED AUCTIONS" << std::endl;

}

void ListAllAuctionsCommand::handle(std::string args){

  (void)args;  // unused - no args

  // Protocol setup
  std::cout << "LISTED ALL AUCTIONS" << std::endl;
}

void ShowAssetCommand::handle(std::string args){

  // Parsing the arguments
  std::stringstream ss(args);
  std::vector<std::string> parsed_args;
  std::string arg;

  while(ss >> arg){
    parsed_args.push_back(arg);
  }

  if(parsed_args.size() != 1){
    std::cout << "[ERROR] Wrong number of arguments" << std::endl;
    return;
  }

  uint32_t a_id = stoi(parsed_args[0]);

  if(a_id > 999 || a_id < 0){
    std::cout << "[ERROR] Incorrect AID." << std::endl;
    return;
  }

  // Protocol setup
  std::cout << "SHOWED IMAGE FILE OF THE ASSET: " << a_id << std::endl;
}

void BidCommand::handle(std::string args){

  // Parsing the arguments
  std::stringstream ss(args);
  std::vector<std::string> parsed_args;
  std::string arg;

  while(ss >> arg){
    parsed_args.push_back(arg);
  }

  if(parsed_args.size() != 2){
    std::cout << "[ERROR] Wrong number of arguments" << std::endl;
    return;
  }

  uint32_t a_id = stoi(parsed_args[0]);
  uint32_t value = stoi(parsed_args[1]);

  if(a_id > 999 || a_id < 0){
    std::cout << "[ERROR] Incorrect AID." << std::endl;
    return;
  }

  // Protocol setup
  std::cout << "BIDDED ON // AUCTION: " << a_id << " // VALUE: " << value << std::endl;
}

void ShowRecordCommand::handle(std::string args){

  // Parsing the arguments
  std::stringstream ss(args);
  std::vector<std::string> parsed_args;
  std::string arg;

  while(ss >> arg){
    parsed_args.push_back(arg);
  }

  if(parsed_args.size() != 1){
    std::cout << "[ERROR] Wrong number of arguments" << std::endl;
    return;
  }

  uint32_t a_id = stoi(parsed_args[0]);

  if(a_id > 999 || a_id < 0){
    std::cout << "[ERROR] Incorrect AID." << std::endl;
    return;
  }

  // Protocol setup
  std::cout << "SHOWED RECORD // AUCTION: " << a_id << std::endl;
}

void LogoutCommand::handle(std::string args){

  (void)args;  // unused - no args

  // Protocol setup
  std::cout << "LOGGED OUT" << std::endl;
}

void UnregisterCommand::handle(std::string args){

  (void)args;  // unused - no args

  // Protocol setup
  std::cout << "UNREGISTERED" << std::endl;
}

void ExitCommand::handle(std::string args){

  (void)args;  // unused - no args

  // Protocol setup
  std::cout << "EXITED" << std::endl;
}