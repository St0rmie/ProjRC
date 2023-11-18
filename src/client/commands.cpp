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

  // Protocol setup
  std::cout << "LOGGED IN //  USER:"<< user_id <<"// PASSWORD: " << password << std::endl;
}