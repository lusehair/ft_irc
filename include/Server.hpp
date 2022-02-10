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

// for_each, min
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
# define KILL "KILL"
# define KICK "KICK"
# define QUIT "QUIT"
# define LIST "LIST"
# define PING "PING"
# define JOIN "JOIN"

# define ERR_NEEDMOREPARAMS "461"
# define ERR_ALREADYREGISTRED "462"     
# define ERR_NOTREGISTERED "451"
# define ERR_NOPRIVILEGES "481"
# define ERR_NOSUCHNICK "401"
# define ERR_CHANOPRIVSNEEDED "482"
# define ERR_NOSUCHCHANNEL "403"
# define ERR_NOTONCHANNEL "442"
# define RPL_LISTSTART "321"
# define RPL_LISTEND "323"

# define CMD_CLOSED_SOCKET true

#include "User.hpp"
#include "log.hpp"
#include "Channel.hpp"

namespace irc
{

    class User;
    class Channel;

    class Server
    {

        private:
            int *                               _password;
            int                                 _listening_socket;
            fd_set                              _client_sockets; // for select parameters
            fd_set                              _ready_sockets; // for select return
            struct                              pending_socket; 
            std::map<int, pending_socket>       _unnamed_users;
            std::map<std::string, User *>       _connected_users;
            std::map<std::string, Channel *>    _running_channels;
            std::map<int, std::string *>        _pending_sends;
            std::set<int>                       _opened_sockets; // set of all opened sockets to easily have the maxfd
            time_t                              _raw_start_time; // used for logs
            struct timeval                      _time_before_timeout; // select timeout
            char                                _main_buffer[MAX_REQUEST_LEN + 1];
            char                                _ip_buffer[INET6_ADDRSTRLEN];
            std::ofstream                       _log_file; 

        public:
            typedef std::map<int, pending_socket>::iterator       unnamed_users_iterator_t;
            typedef std::map<std::string, User *>::iterator       connected_users_iterator_t;
            typedef std::map<std::string, Channel *>::iterator    running_channels_iterator_t;
            typedef std::map<int, std::string *>::iterator    pending_sends_iterator_t;

        private:
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

            bool user_acquired( const int fd );
            void loop( void );

            typedef bool (irc::Server::*command_function)( const int, const std::string, User * );

            void remove_empty_chan( Channel * target_chan );

        private:
            void try_sending_data( void );

            static std::map<const std::string, command_function>    _commands;

            void init_commands_map( void );

            int * pass_hash(std::string input_pass );

            bool cmd_pass( const int input_socket, const std::string command_line, User * input_user);
            bool cmd_nick( const int input_socket, const std::string command_line, User * input_user);
            bool cmd_user( const int input_socket, const std::string command_line, User * input_user);
            bool cmd_ping( const int input_socket, const std::string command_line, User * input_user); 
            bool cmd_kill( const int input_socket, const std::string command_line, User * input_user);
            bool cmd_kick( const int input_socket, const std::string command_line, User * input_user);
            bool cmd_join( const int input_socket, const std::string command_line, User * input_user);
            bool cmd_quit( const int input_socket, const std::string command_line, User * input_user);
            bool cmd_list( const int input_socket, const std::string command_line, User * input_user);
            bool cmd_privmsg(const int input_socket, const std::string command_line, User * input_user);


            template < typename T >
                void cmd_caller( T identifier );

            void send_header(User * input_user) const;
            std::string reply(const User * input_user ,  const char * code, std::string message) const;
            std::string head(const User *input_user);
            void disconnect_user(User * target_user); 

    };

}

#endif
