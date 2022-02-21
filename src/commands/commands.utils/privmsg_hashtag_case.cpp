#include "Server.hpp"

void irc::Server::privmsg_hashtag_case(std::string command_line, User *input_user)
{
    size_t start = command_line.find("#") + 1;
    size_t end = command_line.find(" ", start);
    std::string chan = command_line.substr(start, end - start); 

    running_channels_iterator_t running_channels_iterator =  _running_channels.find(chan);  
    if(running_channels_iterator == _running_channels.end() && command_line.find("NOTICE") != std::string::npos)
    {
        return ;
    }
    if(running_channels_iterator == _running_channels.end())
    {
        input_user->_pending_data._send.append(ERR_NOSUCHCHANNEL(chan));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &input_user->_pending_data._send));
        return ; 
    }
    std::map<User*, const bool>::iterator members_it; 
    for(members_it =  running_channels_iterator->second->_members.begin(); members_it !=  running_channels_iterator->second->_members.end() ; members_it++)
    {
        if(members_it->first != input_user || command_line.find(KICK) != std::string::npos)
        {
            members_it->first->_pending_data._send.append(command_line + "\r\n"); 
            _pending_sends.insert(std::make_pair(members_it->first->_own_socket, &(members_it->first->_pending_data._send)));
        }
    }
    LOG_SENDMSGTOCHAN(_raw_start_time, input_user->_nickname, chan, command_line); 
    return ; 
}
