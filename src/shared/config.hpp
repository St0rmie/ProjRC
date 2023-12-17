#ifndef __CONFIG__
#define __CONFIG__

// Default hostname and port
#define DEFAULT_HOSTNAME "localhost"
#define DEFAULT_PORT     "58086"

// Default arguments sizes
#define USER_ID_SIZE           6  // ist-id
#define AUCTION_ID_SIZE        3
#define MAX_AUCTION_VALUE_SIZE 6
#define MAX_AUCTION_NAME_SIZE  10
#define MAX_FILENAME_SIZE      24
#define MAX_FILE_SIZE          10 * 1000 * 1000  // 10 MB
#define MAX_FILE_SIZE_LENGTH   8
#define MAX_TIMEACTIVE_SIZE    6
#define PASSWORD_SIZE          8
#define MAX_LENGTH_TIMEACTIVE  5
#define MAX_STATUS_SIZE        3
#define PROTOCOL_SIZE          3

// Default udp timeout and max tries
#define UDP_TIMEOUT   5
#define UDP_MAX_TRIES 5

// Default tcp timeout
#define TCP_READ_TIMEOUT_SECONDS   15
#define TCP_READ_TIMEOUT_USECONDS  0
#define TCP_WRITE_TIMEOUT_SECONDS  60
#define TCP_WRITE_TIMEOUT_USECONDS 0

// default buffer lengths
#define UDP_SOCKET_BUFFER_LEN 6002  // Max UDP message size
#define SOCKET_BUFFER_LEN     512   // Max TCP buffer size (512 bytes step)

// Message types
#define TCP_MESSAGE 0
#define UDP_MESSAGE 1

// Max tcp queue size for listen
#define TCP_MAX_QUEUE_SIZE 10

// Default path for client assets
#define CLIENT_ASSET_DEFAULT_PATH ""

#endif