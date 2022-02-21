#include "Server.hpp"

std::string   *irc::Server::cmd_list(const int input_socket, const std::string command_line, User * input_user)
{
    if(input_user == NULL)
    {
        return &_unnamed_users.find(input_socket)->second._pending_data._recv;
    }
    else if(input_user->_already_dead)
    {
        return &input_user->_pending_data._recv;
    }

    std::string ret_list; 
    running_channels_iterator_t running_channel_iterator; 
    size_t cases = command_line.find("#");
    if(cases == std::string::npos)
    {
        for(running_channel_iterator = _running_channels.begin(); running_channel_iterator != _running_channels.end() ; running_channel_iterator++)
        {
            ret_list.append(RPL_LIST(input_user, input_user->_nickname, running_channel_iterator->first) + " " + std::to_string(running_channel_iterator->second->_members_count) + " :\r\n");
        }
       
    }
    else
    {
        size_t pos = 0; 
        while((command_line.find_first_of("#", pos)) != std::string::npos)
        {
            size_t start = pos + 2; 
            size_t end = command_line.find(" ", start); 
            std::string require_channel = command_line.substr(start, end - start); 
            if ((running_channel_iterator = _running_channels.find(require_channel)) != _running_channels.end())
            {
                ret_list.append(RPL_LIST(input_user, input_user->_nickname, running_channel_iterator->first) + " " + std::to_string(running_channel_iterator->second->_members_count) + " :\r\n");
            }
            pos = end; 
        }
    }
    ret_list.append(RPL_LISTEND(input_user, input_user->_nickname)); 
  
    input_user->_pending_data._send.append(ret_list);
    _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
    return &input_user->_pending_data._recv; 
}
