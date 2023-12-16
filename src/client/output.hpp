#ifndef __CLI_OUTPUT__
#define __CLI_OUTPUT__

/**
 * @file output.hpp
 * @brief This file contains the declaration of functions related to printing
 * output in the client application.
 */

#include "shared/protocol.hpp"

// -----------------------------------
// | Command print functions         |
// -----------------------------------

/**
 * @brief Prints the record information.
 * @param aid The auction ID.
 * @param message The server's show record message.
 */
void printRecord(std::string aid, ServerShowRecord message);

/**
 * @brief Prints the open auction information.
 * @param message The server's open auction message.
 */
void printOpenAuction(ServerOpenAuction message);

/**
 * @brief Prints the close auction information.
 * @param message The client's close auction message.
 */
void printCloseAuction(ClientCloseAuction message);

/**
 * @brief Prints the asset information.
 * @param message_c The client's show asset message.
 * @param message_s The server's show asset message.
 */
void printShowAsset(ClientShowAsset message_c, ServerShowAsset message_s);

/**
 * @brief Prints the bid information.
 * @param message The client's bid message.
 */
void printBid(ClientBid message);

// -----------------------------------
// | Basic Functions                 |
// -----------------------------------

/**
 * @brief Prints an error message.
 * @param str The error message.
 */
void printError(std::string str);

/**
 * @brief Prints a success message.
 * @param success_description The description of the success.
 */
void printSuccess(std::string success_description);

#endif