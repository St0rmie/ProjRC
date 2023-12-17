# Project RC - Group 86

RC @ IST - Project 2023/2024

## Compiling

The project can be compiled by running `make` in this directory.
This project uses C++17.
This project uses a non default library -lreadline which has been tested on Sigma.
In the case the above library is missing, the execution of the following command is recommended:

```
sudo apt-get install libreadline-dev
```

Once compiled, two binaries, `user` and `AS` will appear in this directory:

- `user` : the binary that represents the client to make requests to the AS.
- `AS` : the auction server.

## Client (user)

When executing the `user`, there are two flags that can be useful:

- `-n <hostname>` : defines the hostname of the server.
- `-p <port>` : defines the port of the server.

Once the client is running, it will wait for the user to input a command.
The list of commands can be shown by executing the command `help` by typing it into the prompt.

The client will mantain a session state (if a user is logged in or not) while it is running but, if the client goes down by typing the command `help`, it automatically issues a `logout` command to the server and that session is wiped locally.

The client features terminal history and tab completion which are implemented by using de "readline" and "history" libraries.

The assets don't have a particular folder to be stored while executing the `show_asset` command. However, that can be changed in the `config.hpp` file in the `shared` folder by changing the variable `CLIENT_ASSET_DEFAULT_PATH` to the folder path.

## Server (AS)

When executing the `AS`, there are two flags that can be useful:

- `-p <port>` : defines the port of the server.
- `-v` : verbose mode.

The verbose mode is a mode where the AS outputs to the screen a short description of the received requests (UID, type
of request) and the IP and port originating those requests. In our implementation we decided to include a snippet of 100 bytes of the sent message too because we thought it would be useful for debug.

The server handles the SIGINT (CTRL + C) signal by changing a global variable that will be evaluated back on the normal program and will start a controlled close and free of all elements needed.

We used fork() for concurrency because it would be more resilient if one of the workers fails. In our case the main server process branches into two: processUDP and processTCP. processUDP receives one message at a time but due to the way UDP works it can handle it well. processTCP creates a new child process (processTCPChild) whenever it receives a message so that the child can handle it.

The system supports the maximum of 10 TCP child processes running but that can be changed on the `config.hpp` file in the `shared` folder by changing the variable `TCP_MAX_QUEUE_SIZE`.

The server uses a database that will be further described next.

## Database

The database is initialized as a folder in the same directory as the `AS` is running. It keeps the data between sessions and the best way to reset the database is to execute the command:

```
make clean-database
```

We used one named semaphore for synchronization and it has a unique name binded to the port number so that several auction servers can be running in the same machine without conflicts.

## File structure of the project

The project is divided in three different folders:

- `client` : specific files for the `user` binary
- `server` : specific files for the `AS` binary
- `shared` : common files between both binaries.

## Changeable constants

The `config.hpp` file in the `shared` folder has some other constants that can be changed such as timeouts for UDP and TCP reads and writes, default hostname and port, buffer sizes, argument sizes, etc.
