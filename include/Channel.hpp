#ifndef CHANNEL_HPP
# define CHANNEL_HPP

// std::map
#include <map>

// std::string
#include <string>

// irc::User
#include "User.hpp"
#include "Server.hpp"

namespace irc
{

    class Server;
    class User;

    class Channel
    {

        private:
            Server &                            _server_master;
            std::map<const User *, const bool>  _members;
            const std::string                   _name;
            // std::string                         _topic;
            int                                 _members_count;

            Channel();
            Channel(const Channel & other);
            Channel & operator = (const Channel & other);

        public:
            Channel(irc::Server & server, const irc::User * channel_operator, const std::string channel_name);

            ~Channel();

            const std::string & get_name( void ) const;

            void add_user(const irc::User * new_member);
            void kick_user(irc::User * target_member);

    };

}

#endif
