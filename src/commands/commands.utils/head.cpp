#include "Server.hpp"

/**
 * @brief Header Generator for the reply to the client 
 * 
 * @param input_user 
 * @return std::string 
 */
std::string irc::Server::head(const User *input_user)
{
    std::string ret =  ":" + input_user->_nickname + "!" + input_user->_username + "@" + _hostname + " ";
    return (ret);
}
