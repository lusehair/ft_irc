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
            // std::string                         _topic;
           
            Channel();
            Channel(const Channel & other);
            Channel & operator = (const Channel & other);

        public:
            std::map< User *, const bool>  _members;
             int                                 _members_count;


            Channel(irc::Server & server, irc::User * channel_operator, const std::string channel_name);

            ~Channel();

            const std::string & get_name( void ) const;
            int  get_members_count( void ) const; 
            std::string const getName(); 
            void add_user(irc::User * new_member);
            void kick_user(irc::User * target_member);

    };

}

#endif
