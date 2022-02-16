#ifndef USER_HPP
# define USER_HPP

// For string storage (username)
# include <string>
# include <list> // For channel container
# include "Channel.hpp"

namespace irc
{

    class Channel;

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
            std::list<Channel *>   _joined_channels; 
            User();

        public:
            User(const std::string nick, const std::string user, const int fd);
            ~User();

            void make_current(Channel * current_channel);
            void remove_channel(Channel * to_remove_channel);

    };

}

#endif
