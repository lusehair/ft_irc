#ifndef CHANNEL_HPP
# define CHANNEL_HPP

// std::map
#include <map>

// std::string
#include <string>

// irc::User
#include "User.hpp"

namespace irc
{

    class Channel
    {

        private:
            std::map<irc::User *, const bool>   _members;
            const std::string                   _name;
            int                                 _members_count;

            Channel();
            Channel(const Channel & other);
            Channel & operator = (const Channel & other);

        public:
            Channel(const User * operator, const std::string channel_name);

            ~Channel();

            void add_user(const User * new_member);
            void kick_user(const User * target_member);

    };

}

irc::Channel::Channel( const User * operator, const std::string channel_name )
    : _name(channel_name)
{
    _members.insert(make_pair(operator, true));
    ++_members_count;
}

void
irc::Channel::add_user( const User * new_member )
{
    _members.insert(make_pair(new_member, false));
    ++_members_count;
}

void
irc::Channel::kick_user( const User * target_member )
{
    _members.erase(target_member);
    --_members_count;
    if (_members_count == 0)
    {
        ;
    }
}

#endif
