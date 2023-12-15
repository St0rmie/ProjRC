#ifndef __CONFIG__
#define __CONFIG__

#define DEFAULT_HOSTNAME "localhost"
#define DEFAULT_PORT     "58086"

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

#define CODE_LOGIN_USER         "LIN"
#define CODE_LOGIN_SERVER       "RLI"
#define CODE_LOGOUT_USER        "LOU"
#define CODE_LOGOUT_SERVER      "RLO"
#define CODE_UNREGISTER_USER    "UNR"
#define CODE_UNREGISTER_SERVER  "RUR"
#define CODE_LIST_AUC_USER      "LMA"
#define CODE_LIST_AUC_SERVER    "RMA"
#define CODE_LIST_MYB_USER      "LMB"
#define CODE_LIST_MYB_SERVER    "RMB"
#define CODE_LIST_ALLAUC_USER   "LST"
#define CODE_LIST_ALLAUC_SERVER "RLS"
#define CODE_SHOWREC_USER       "SRC"
#define CODE_SHOWREC_SERVER     "RRC"
#define CODE_OPEN_AUC_CLIENT    "OPA"
#define CODE_OPEN_AUC_SERVER    "ROA"
#define CODE_CLOSE_AUC_CLIENT   "CLS"
#define CODE_CLOSE_AUC_SERVER   "RCL"
#define CODE_SHOW_ASSET_CLIENT  "SAS"
#define CODE_SHOW_ASSET_SERVER  "RSA"
#define CODE_BID_CLIENT         "BID"
#define CODE_BID_SERVER         "RBD"
#define CODE_ERROR              "ERR"

#define SERVER_UDP_TIMEOUT 300
#define UDP_TIMEOUT        5
#define UDP_MAX_TRIES      5

#define TCP_READ_TIMEOUT_SECONDS   15
#define TCP_READ_TIMEOUT_USECONDS  0
#define TCP_WRITE_TIMEOUT_SECONDS  60
#define TCP_WRITE_TIMEOUT_USECONDS 0

#define UDP_SOCKET_BUFFER_LEN 6002  // Max UDP message size
#define SOCKET_BUFFER_LEN     512

#define TCP_MESSAGE 0
#define UDP_MESSAGE 1

#define TCP_MAX_QUEUE_SIZE 8

#define CLIENT_ASSET_DEFAULT_PATH "assets/"

#endif