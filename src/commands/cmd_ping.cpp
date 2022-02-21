#include "Server.hpp"

/**
 * @brief Response to ping client (pong)
 * 
 * @param input_socket 
 * @param command_line 
 * @param input_user 
 * @return std::string* 
 */
std::string *    irc::Server::cmd_ping(const int input_socket, const std::string command_line, User * input_user)
{
    if(input_user == NULL)
    {
        LOG_PONGNOREGISTERUSER(_raw_start_time, input_socket);
        unnamed_users_iterator_t current_unnamed_user = _unnamed_users.find(input_socket);
        current_unnamed_user->second._pending_data._send.append(ERR_NOTREGISTERED);
        return &current_unnamed_user->second._pending_data._recv; 
    }
    if(input_user->_already_dead)
    {
        return &input_user->_pending_data._recv;
    }

    std::string ret = head(input_user) + "PONG :" + command_line.substr(command_line.find(" ") + 1, command_line.size() - command_line.find(" ") + 1) + "\r\n";  
    LOG_PONGUSERPING(_raw_start_time, input_user->_nickname);
    
    input_user->_pending_data._send.append(ret);
    _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
    
    return &input_user->_pending_data._recv;
} 
