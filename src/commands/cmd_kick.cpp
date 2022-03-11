#include "Server.hpp"

std::string *
irc::Server::cmd_kick(const int input_socket, const std::string command_line, User * input_user)
{
    if(input_user == NULL)
    {
        unnamed_users_iterator_t current_unnamed_user = _unnamed_users.find(input_socket);
        current_unnamed_user->second._pending_data._send.append(ERR_NOTREGISTERED);
        _pending_sends.insert(std::make_pair(input_socket, &current_unnamed_user->second._pending_data._send));
        return &current_unnamed_user->second._pending_data._recv;
    }
    else if(input_user->_already_dead)
    {
        return &input_user->_pending_data._recv;
    }

    if (std::count(command_line.begin(), command_line.end(), ' ') < 2)
    {
        input_user->_pending_data._send.append(ERR_NEEDMOREPARAMS(input_user->_nickname, KICK)); 
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
        return &input_user->_pending_data._recv;
    }

    size_t start = command_line.find("#") + 1; 
    size_t end = command_line.find(" ", start); 
    std::string channel = command_line.substr(start, end - start); 
    running_channels_iterator_t running_channel_iterator = _running_channels.find(channel); 
    if(running_channel_iterator == _running_channels.end())
    {
        input_user->_pending_data._send.append(ERR_NOSUCHCHANNEL(channel)); 
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
        return &input_user->_pending_data._recv;
    }

    std::map<User *, const bool>::iterator kicking_user_iterator = running_channel_iterator->second->_members.find(input_user);
    if(kicking_user_iterator == running_channel_iterator->second->_members.end())
    {
        input_user->_pending_data._send.append(ERR_NOTONCHANNEL(channel));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
        return &input_user->_pending_data._recv;
    }
    if(kicking_user_iterator->second != true)
    {
        input_user->_pending_data._send.append(ERR_CHANOPRIVSNEEDED(channel)); 
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
        return &input_user->_pending_data._recv;
    }

    std::string ret; 
    std::string target_nickname;

    start = end + 1; 
    end = command_line.find(" ", start);
    target_nickname = command_line.substr(start, end - start);
    connected_users_iterator_t user_target_iterator = _connected_users.find(target_nickname);
    if (user_target_iterator != _connected_users.end())
    {
        std::map<User *, const bool>::iterator user_iterator = running_channel_iterator->second->_members.find(user_target_iterator->second);
        if (user_iterator != running_channel_iterator->second->_members.end())
        {
            privmsg_hashtag_case(head(input_user) + command_line, input_user); 
            running_channel_iterator->second->kick_user(user_iterator->first);
            return &input_user->_pending_data._recv;
        }
    }
    input_user->_pending_data._send.append(ERR_USERNOTINCHANNEL(input_user->_nickname, target_nickname, channel)); 
    _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send))); 
    return &input_user->_pending_data._recv;
}
