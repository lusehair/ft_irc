#include "Server.hpp"

std::string *irc::Server::cmd_notice(const int input_socket, const std::string command_line, User *input_user)
{
    if(input_user == NULL) 
    {
        return &_unnamed_users.find(input_socket)->second._pending_data._recv;
    }
    else if(input_user->_already_dead)
    {
       return &_unnamed_users.find(input_socket)->second._pending_data._recv; 
    }

    size_t start = command_line.find(" ") + 1;
    size_t end = command_line.find(":") - 1;
    
    std::string sender = input_user->_nickname; 
    std::string reciever = command_line.substr(start, end - start);
    std::string ret = head(input_user) + command_line + "\r\n"; 
    
    if(command_line.find("#") != std::string::npos)
    {
        privmsg_hashtag_case(ret, input_user); 
        return &input_user->_pending_data._recv; 
    }

    std::map<std::string , User * >::iterator user_it = _connected_users.find(reciever);
    if(user_it == _connected_users.end())
    {
        return &input_user->_pending_data._recv;         
    }

    user_it->second->_pending_data._send.append(ret);
    _pending_sends.insert(std::make_pair(user_it->second->_own_socket, &(user_it->second->_pending_data._send)));
    return &input_user->_pending_data._recv;
}
