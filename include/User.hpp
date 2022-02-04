#ifndef USER_HPP
# define USER_HPP

// For string storage (username)
# include <string>
# include <list> // For channel container
// # include "Channel.hpp"

namespace irc
{

    class User
    {

        private:
            struct partial_packet
            {
                std::string _recv;
                std::string _send;
            };

        public:
            int                 _own_socket;
            std::string         _nickname;
            std::string         _username;
            bool                _isOperator;
            partial_packet      _pending_data;

        private:
            std::list<void *>   _own_chan; // void * = const Channel *

            User();

        public:
            User(const std::string nick, const std::string user, const int fd);
            // User(const User * other);

            ~User();

    };

}

#endif
