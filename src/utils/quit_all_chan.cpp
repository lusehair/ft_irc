#include "Server.hpp"

void
irc::Server::quit_all_chan(User * target, std::string & reason)
{
    std::string killed_notice = MSG_QUIT(target, reason);
    std::list<Channel *>::iterator current_channel = target->_joined_channels.begin();
    if (current_channel != target->_joined_channels.end())
    {
        std::list<Channel *>::iterator tmp = current_channel;
        ++current_channel;
        for(std::map<User *, const bool>::iterator members_it = (*tmp)->_members.begin();
                members_it != (*tmp)->_members.end();
                ++members_it)
        {
            members_it->first->_pending_data._send.append(killed_notice);
            _pending_sends.insert(std::make_pair(members_it->first->_own_socket, &(members_it->first->_pending_data._send)));
        }
        LOG_LEFTCHAN(_raw_start_time, target->_nickname, "from quitting all chan");
        (*tmp)->kick_user(target);
    }
}
