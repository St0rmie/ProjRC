#ifndef __CONFIG__
#define __CONFIG__

#define DEFAULT_HOSTNAME "localhost"
#define DEFAULT_PORT     "58086"

#define USER_ID_SIZE          6
#define AUCTION_ID_SIZE       3
#define AUCTION_VALUE_SIZE    5
#define MAX_AUCTION_NAME_SIZE 1024
#define MAX_FILENAME_SIZE     1024
#define MAX_TIMEACTIVE_SIZE   6
#define PASSWORD_SIZE         8
#define MAX_LENGTH_TIMEACTIVE 20

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

#define UDP_TIMEOUT   30
#define UDP_MAX_TRIES 5

#define TCP_READ_TIMEOUT_SECONDS   20
#define TCP_READ_TIMEOUT_USECONDS  0
#define TCP_WRITE_TIMEOUT_SECONDS  60
#define TCP_WRITE_TIMEOUT_USECONDS 0

#define UDP_SOCKET_BUFFER_LEN 32768
#define SOCKET_BUFFER_LEN     512

#endif