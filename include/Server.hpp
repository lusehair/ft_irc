#ifndef SERVER_HPP
# define SERVER_HPP

// memset, strerro << "\n"r
#include <cstring>

// data storage
# include <string>
# include <map>
# include <set>

// fd, input and output
# include <iostream>
# include <unistd.h>

// INET6_ADDRSTRLEN
#include <netinet/in.h>

// accept, bind, setsockopt, recv, send, socket, socket option macro
# include <sys/socket.h>

// fcntl
#include <fcntl.h>

// gai_strerror, addrinfo, AI_NUMERICHOST
# include <netdb.h>

// time_t (used to print timed logs)
# include <ctime>

// struct timeval (select last parameter)
# include <sys/time.h>

// errno for the logs
# include <cerrno>

// make_pair
# include <utility>

// for_each
# include <algorithm>

// Size of the receiving buffer being 512 as the max size of a packet defined in rfc 2812
# define BUFFER_SIZE 512
// Also named backLog, use in listen to limit the number of simultaneous client connections
# define MAX_PENDING_CONNECTIONS 10

#include "User.hpp"

namespace irc
{

    class Server
    {

        private:
            std::string                     _password;
            int                             _listening_socket;
            fd_set                          _client_sockets; // for select parameters
            fd_set                          _ready_sockets; // for select return
            std::map<std::string, User *>   _connected_users;
            std::map<int, std::string>      _unnamed_users;
            std::set<void *>                _running_channels; // void * == Channel; int == Modes
            std::set<int>                   _opened_sockets; // set of all opened sockets to easily have the maxfd
            time_t                          _raw_start_time; // used for logs
            struct timeval                  _time_before_timeout; // select timeout
            char                            _main_buffer[BUFFER_SIZE];
            char                            _ip_buffer[INET6_ADDRSTRLEN];

            // Do not allow external use of default ctor, copy ctor and assignation
            Server();
            Server(const Server & other);
            Server & operator = (const Server & other);

            struct CtorException : std::exception
            {
                const char * what() const _NOEXCEPT
                {
                    return "CtorException, check the logs!\n";
                }
            };

        public:
            // Only ctor actually used
            Server( char * port_number ); // TODO: add password as a second parameter

            ~Server();

            void set_password( const std::string new_password );

            bool user_acquired(const int fd);
            void loop( void );

        private:
            typedef void (* command_function)(const std::string);
            static std::map<const std::string, command_function>    _commands;

    };

}

#endif
