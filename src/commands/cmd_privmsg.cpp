#include "Server.hpp"

/**
 * @brief the PRIVMSG or /msg on irssi is, to send a private message to an another user 
 * 
 * @param input_socket 
 * @param command_line 
 * @param input_user 
 * @return std::string* 
 */
std::string *    irc::Server::cmd_privmsg(const int input_socket, const std::string command_line, User *input_user)
{
    if(input_user == NULL)
    {
        unnamed_users_iterator_t current_unnamed_user = _unnamed_users.find(input_socket);
        current_unnamed_user->second._pending_data._send.append(ERR_NOTREGISTERED);
        _pending_sends.insert(std::make_pair(input_socket, &current_unnamed_user->second._pending_data._recv));
        return &current_unnamed_user->second._pending_data._recv;
    }
    else if(input_user->_already_dead)
    {
        return &input_user->_pending_data._recv;
    }

    if (std::count(command_line.begin(), command_line.end(), ' ') < 2)
    {
        input_user->_pending_data._send.append(ERR_NOTEXTTOSEND); 
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
        return &input_user->_pending_data._recv;
    }

    size_t start = command_line.find(" ") + 1;
    size_t end = command_line.find(":") - 1;
    
    std::string sender = input_user->_nickname; 
    std::string reciever = command_line.substr(start, end - start);
    std::string ret = head(input_user) + command_line; 
    
    if(command_line.find("#") != std::string::npos)
    {
        privmsg_hashtag_case(ret, input_user);
        return &input_user->_pending_data._recv;
    }
    
    std::map<std::string , User * >::iterator user_it = _connected_users.find(reciever);
    if(user_it == _connected_users.end())
    {
        ret = ERR_NOSUCHNICK(reciever);
        input_user->_pending_data._send.append(ret);
        _pending_sends.insert(std::make_pair(input_socket, &(input_user->_pending_data._send)));
        return &input_user->_pending_data._recv;         
    }

    user_it->second->_pending_data._send.append(ret + "\r\n");
    _pending_sends.insert(std::make_pair(user_it->second->_own_socket, &(user_it->second->_pending_data._send)));
    return &input_user->_pending_data._recv;
}
