#include "Server.hpp"

/**
 * @brief this function is use for generate the header message when a new user enter in a channel 
 * 
 * @param input_user 
 * @param channel_target 
 */
void irc::Server::send_names(User * input_user, Channel * channel_target)
{
    std::string ret = head(input_user) + "353 " + input_user->_nickname + " = #" + channel_target->getName()+ " :";
    std::string notify = head(input_user) + "JOIN :#" + channel_target->getName() + "\r\n";

    std::map<User*, const bool>::iterator members_it;
    for(members_it =  channel_target->_members.begin(); members_it !=  channel_target->_members.end() ; members_it++)
    {
        if(members_it->second == false)
        {
            ret.append(" " + members_it->first->_nickname);
        }
        else 
        {
            ret.append(" @" + members_it->first->_nickname);
        }

        members_it->first->_pending_data._send.append(notify);
        _pending_sends.insert(std::make_pair(members_it->first->_own_socket, &(members_it->first->_pending_data._send)));
    }
    ret.append("\r\n");

    input_user->_pending_data._send.append(ret + RPL_ENDOFNAMES(input_user, input_user->_nickname, channel_target->getName())); 
    _pending_sends.insert(std::make_pair(input_user->_own_socket, &(input_user->_pending_data._send)));
}
