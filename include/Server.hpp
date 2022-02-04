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
#include <fstream>


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


# include <fstream>


// Size of the receiving buffer being 512 as the max size of a packet defined in rfc 2812
# define MAX_REQUEST_LEN 512
// Also named backLog, use in listen to limit the number of simultaneous client connections
# define MAX_PENDING_CONNECTIONS 10

# define PASS "PASS"
# define NICK "NICK"
# define USER "USER"
# define PRIVMSG "PRIVMSG"
#define KILL "KILL"

# define ERR_NEEDMOREPARAMS "461"
# define ERR_ALREADYREGISTRED "462"     
# define ERR_ERR_NOTREGISTERED "451"
# define ERR_NOPRIVILEGES "481"
# define ERR_NOSUCHNICK "401"


#include "User.hpp"
#include "log.hpp"

namespace irc
{

    class Server
    {

        private:
            int *                           _password;
            int                             _listening_socket;
            fd_set                          _client_sockets; // for select parameters
            fd_set                          _ready_sockets; // for select return
            struct                          pending_socket; 
            std::map<int, pending_socket>   _unnamed_users;
            std::map<std::string, User *>   _connected_users;
            std::set<void *>                _running_channels; // void * == Channel; int == Modes
            std::set<int>                   _opened_sockets; // set of all opened sockets to easily have the maxfd
            time_t                          _raw_start_time; // used for logs
            struct timeval                  _time_before_timeout; // select timeout
            char                            _main_buffer[MAX_REQUEST_LEN + 1];
            char                            _ip_buffer[INET6_ADDRSTRLEN];
            std::ofstream                   _log_file; 

            struct CtorException : std::exception
            {
                const char * what() const _NOEXCEPT
                {
                    return "CtorException, check the logs!\n";
                }
            };

            struct partial_packet
            {
                std::string _recv;
                std::string _send;
            };

            // user_name??
            struct pending_socket
            {
                std::string     nick_name;
                bool            pass_check;
                partial_packet  _pending_data;

                pending_socket()
                    : nick_name(std::string())
                    , pass_check(false)
                {
                }
            };
            

        public:
            // Only ctor actually used
            Server( const char * port_number, const char * input_pass ); // TODO: add password as a second parameter

            ~Server();

            void set_password( const std::string new_password );

            bool user_acquired(const int fd);
            void loop( void );

            typedef void (irc::Server::*command_function)( const int, const std::string, User * );

        private:

            static std::map<const std::string, command_function>    _commands;

            void init_commands_map( void );

            int * pass_hash(std::string input_pass );

            void cmd_pass( const int input_socket, const std::string command_line, User * input_user);
            void cmd_nick( const int input_socket, const std::string command_line, User * input_user);
            void cmd_user( const int input_socket, const std::string command_line, User * input_user);
            void cmd_pong( const int input_socket, const std::string command_line, User * input_user); 
            void cmd_kill( const int input_socket, const std::string command_line, User * input_user);


            template < typename T >
                void cmd_caller( T identifier );

            void send_header(const User * input_user) const;

            void disconnect_user(User * target_user); 

    };

}

#endif
