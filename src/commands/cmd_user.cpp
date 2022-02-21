#include "Server.hpp"

/**
 * @brief command USER from IRC Protocl
 * 
 * We have 2 cases : 
 * 1. If the USER command doesn't have username on his request  (461)
 * 2. User is already register (462)
 * 
 * @param input_socket (int)
 * @sa RFC 2812 (3.1.3)
 * 
 */
std::string * irc::Server::cmd_user(const int input_socket, const std::string command_line, User * input_user)
{
    if(input_user != NULL)
    {
        LOG_USERTAKEN(_raw_start_time, input_user->_nickname, input_user->_username);
        input_user->_pending_data._send.append(ERR_ALREADYREGISTRED(input_user->_nickname));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
        return &input_user->_pending_data._recv;
    }

    unnamed_users_iterator_t current_unnamed_user = _unnamed_users.find(input_socket);

    if (current_unnamed_user->second.pass_check != true)
    {
        _unnamed_users.erase(current_unnamed_user);
        _opened_sockets.erase(current_unnamed_user->first);
        _pending_sends.erase(current_unnamed_user->first);
        FD_CLR(current_unnamed_user->first, &_client_sockets);
        close(current_unnamed_user->first);
        return NULL;
    }

    std::size_t nb_of_space = std::count(command_line.begin(), command_line.end(), ' ');
    if((nb_of_space < 4 || command_line.find(':') == std::string::npos))
    {
        LOG_NOPARAM(_raw_start_time, input_socket, command_line);
        return &current_unnamed_user->second._pending_data._recv;
    }

    std::size_t start = strlen(USER) + 1; 
    std::size_t end = command_line.find(' ', start); 
    
    current_unnamed_user->second.username = command_line.substr(start, end - start);
    if (!current_unnamed_user->second.nickname.empty()) 
    {
        return (user_create(current_unnamed_user));
    }
    return (&current_unnamed_user->second._pending_data._recv);
}
