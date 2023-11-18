#include "client.hpp"
#include "commands.hpp"

#include <string>
#include <iostream>
#include <memory>

int main(int argc, char *argv[]){

    CommandManager commandManager;
    registerCommands(commandManager);

    while(!std::cin.eof()){
        commandManager.waitCommand();
    }

    std::cout << "Bye Bye!" << std::endl;
}

void registerCommands(CommandManager &manager){
    manager.registerCommand(std::make_shared<LoginCommand>());
}