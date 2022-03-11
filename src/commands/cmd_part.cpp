#include "Server.hpp"

void
irc::Server::make_user_part(User * input_user, const std::string channel_name, const std::string reason)
{
    std::string channel_part_notice = PART_NOTICE(input_user, channel_name, reason);
    running_channels_iterator_t running_channels_iterator;
    running_channels_iterator = _running_channels.find(channel_name);

    if (running_channels_iterator != _running_channels.end())
    {
        if (running_channels_iterator->second->_members.find(input_user)
                != running_channels_iterator->second->_members.end())
        {
            running_channels_iterator_t tmp = running_channels_iterator;
            ++running_channels_iterator;
            for(std::map<User *, const bool>::iterator members_it = tmp->second->_members.begin();
                    members_it !=  tmp->second->_members.end();
                    ++members_it)
            {
                members_it->first->_pending_data._send.append(channel_part_notice);
                _pending_sends.insert(std::make_pair(members_it->first->_own_socket, &(members_it->first->_pending_data._send)));
            }
            LOG_LEFTCHAN(_raw_start_time, input_user->_nickname, channel_name);
            tmp->second->kick_user(input_user);
        }
        else
        {
            input_user->_pending_data._send.append(ERR_NOTONCHANNEL(channel_name));
            _pending_sends.insert(std::make_pair(input_user->_own_socket, &input_user->_pending_data._send));
        }
    }
    else
    {
        input_user->_pending_data._send.append(ERR_NOSUCHCHANNEL(channel_name));
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &input_user->_pending_data._send));
    }
}

std::string *    irc::Server::cmd_part(const int input_socket, const std::string command_line, User * input_user)
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

    if (std::count(command_line.begin(), command_line.end(), ' ') < 1)
    {
        input_user->_pending_data._send.append(ERR_NEEDMOREPARAMS(input_user->_nickname, PART)); 
        _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
        return &input_user->_pending_data._recv;
    }

    size_t next_comma = 0;
    size_t reason_begin = command_line.find(':');
    std::string reason;
    if (reason_begin != command_line.npos)
    {
        reason = command_line.substr(reason_begin - 1);
    }
    else
    {
        reason = + " :" + input_user->_nickname + "\r\n";
    }

    std::string channel_name;
    while ((next_comma = command_line.find(',', next_comma)) < reason_begin)
    {
        channel_name = command_line.substr(command_line.find_last_of(", ", next_comma) + 1, next_comma);
        make_user_part(input_user, channel_name, reason);
        ++next_comma;
    }
    channel_name = command_line.substr(command_line.find_last_of(", ", next_comma) + 2, next_comma);
    make_user_part(input_user, channel_name, reason);
    return &input_user->_pending_data._recv;
}
