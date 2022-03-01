#include "Server.hpp"

/**
 * @brief Command NICK from IRC Protocol 
 * 
 * 
 * We have 4 cases 
 * 1. If User want changes his nickname to an another (same FD)
 * 2. If the User have the same nickname as another, kick the two users
 * 3. If it's a new user, we put the nickname in unnamed_users map.
 * 4. If the nickname is not in the block_list
 * 
 * @param const int input_socket 
 *@sa RFC 2812 (3.1.2)
 */
std::string * irc::Server::cmd_nick(const int input_socket, const std::string command_line, User * input_user)
{
    if (input_user != NULL)
    {
        if (std::count(command_line.begin(), command_line.end(), ' ') < 1)
        {
            input_user->_pending_data._send.append(ERR_NEEDMOREPARAMS(input_user->_nickname, NICK)); 
            _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
            return &input_user->_pending_data._recv;
        }

        std::string nick = command_line.substr(strlen(NICK) + 1);

        if(_connected_users.find(nick) != _connected_users.end())
        {
            LOG_NICKTAKEN(_raw_start_time,input_user->_nickname, nick);
            input_user->_pending_data._send.append(ERR_NICKNAMEINUSE(input_user->_nickname, nick));
            _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
        }
        else
        {
            input_user->_pending_data._send.append(head(input_user) + " " + NICK + " " + nick + "\r\n");
            _pending_sends.insert(std::make_pair(input_socket, &(input_user->_pending_data._send)));

            _connected_users.erase(input_user->_nickname);

            LOG_NICKCHANGE(_raw_start_time, input_user->_nickname, nick);

            input_user->_nickname = nick;
            _connected_users.insert(std::make_pair(input_user->_nickname, input_user));
            return &input_user->_pending_data._recv;
        }
       
       return &input_user->_pending_data._recv;
    }
    else
    {
        unnamed_users_iterator_t current_unnamed_user = _unnamed_users.insert(std::make_pair(input_socket, pending_socket())).first;

        if (std::count(command_line.begin(), command_line.end(), ' ') < 1)
        {
            current_unnamed_user->second._pending_data._send.append(ERR_NEEDMOREPARAMS(current_unnamed_user->second.nickname, NICK)); 
            _pending_sends.insert(std::make_pair(input_socket, &(current_unnamed_user->second._pending_data._send)));
            return &current_unnamed_user->second._pending_data._recv;
        }

        std::string nick = command_line.substr(strlen(NICK) + 1);

        if(_connected_users.find(nick) != _connected_users.end())
        {
            current_unnamed_user->second._pending_data._send.append(": 433 * " + nick + " :Nickname is already in use\r\n"); 
            _pending_sends.insert(std::make_pair(input_socket, &(current_unnamed_user->second._pending_data._send)));
            return &current_unnamed_user->second._pending_data._recv;
        }

        if (current_unnamed_user->second.pass_check != true) 
        {
            _unnamed_users.erase(current_unnamed_user);
            _opened_sockets.erase(current_unnamed_user->first);
            _pending_sends.erase(current_unnamed_user->first);
            FD_CLR(input_socket, &_client_sockets);
            close(input_socket);
            return NULL;
        }

        current_unnamed_user->second.nickname = nick;
        LOG_NICKREGISTER(_raw_start_time, nick);

        if (!current_unnamed_user->second.username.empty())
        {
            return (user_create(current_unnamed_user));
        }
        return &current_unnamed_user->second._pending_data._recv;
    }
}
