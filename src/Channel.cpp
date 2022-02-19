#include "Channel.hpp"

irc::Channel::Channel( irc::Server & server, User * channel_operator, const std::string channel_name )
    : _server_master(server)
    , _name(channel_name)
    , _members_count(0)
{
    _members.insert(std::make_pair(channel_operator, true));
    ++_members_count;
}

irc::Channel::~Channel()
{
}

const std::string &
irc::Channel::get_name( void ) const
{
    return (_name);
}





void
irc::Channel::add_user( User * new_member )
{
    _members.insert(std::make_pair(new_member, false));
    
    ++_members_count;
}

void
irc::Channel::kick_user( User * target_member )
{
    _members.erase(target_member);
    target_member->remove_channel(this);
    --_members_count;
    if (_members_count == 0)
    {
        _server_master.remove_empty_chan(this);
    }
}

std::string const 
irc::Channel::getName(void)
{
    return (_name);
}

int 
irc::Channel::get_members_count( void ) const
{
    return (_members_count);
}