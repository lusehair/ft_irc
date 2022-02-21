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
            const std::string                   _name;
            
            Channel();
            Channel(const Channel & other);
            Channel & operator = (const Channel & other);

        public:
            std::map<User *, const bool>  _members;

            Channel(irc::Server & server, irc::User * channel_operator, const std::string channel_name);

            ~Channel();
            int                                 _members_count;
            const std::string & get_name( void ) const;
            std::string const getName(); 
            void add_user(irc::User * new_member);
            void kick_user(irc::User * target_member);

    };

}

#endif
