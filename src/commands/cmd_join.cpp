#include "Server.hpp"

/**
 * @brief JOIN command is use to create a join an existing channel 
 * 
 * @param input_socket 
 * @param command_line 
 * @param input_user 
 * @return std::string* 
 */
std::string *    irc::Server::cmd_join(const int input_socket, const std::string command_line, User * input_user)
{
    if(input_user == NULL)
    {
        return &_unnamed_users.find(input_socket)->second._pending_data._recv; 
    }
    else if(input_user->_already_dead)
    {
        return &input_user->_pending_data._recv;
    }

    size_t next_comma, next_space, next_hashtag = 0;
    std::string channel_name;
    running_channels_iterator_t running_channels_iterator;

    while ((next_hashtag = command_line.find('#', next_hashtag)) != command_line.npos) 
    {
        next_space = command_line.find(' ', next_hashtag);
        next_comma = command_line.find(',', next_hashtag);
        channel_name = command_line.substr(next_hashtag + 1, std::min(std::min(next_space, next_comma), command_line.length()) - (next_hashtag + 1));
        running_channels_iterator = _running_channels.find(channel_name);
        if (running_channels_iterator != _running_channels.end())
        {
            LOG_JOINCHAN(_raw_start_time, input_user->_nickname, channel_name); 
            running_channels_iterator->second->add_user(input_user);
        } 
        else 
        {
            running_channels_iterator = _running_channels.insert(std::make_pair(channel_name, new irc::Channel(*this, input_user, channel_name))).first;
            LOG_CREATECHAN(_raw_start_time, input_user->_nickname, channel_name);
        }
        
        input_user->make_current(running_channels_iterator->second);

        ++next_hashtag;
        send_names(input_user, running_channels_iterator->second);
    }
    return &input_user->_pending_data._recv;
}
