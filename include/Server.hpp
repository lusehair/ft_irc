#ifndef SERVER_HPP
# define SERVER_HPP

// memset, strerro << "\n"r
# include <cstring>

// data storage
# include <string>
# include <map>
# include <set>
# include <vector>

// fd, input and output
# include <iostream>
# include <unistd.h>
# include <fstream>

// INET6_ADDRSTRLEN
# include <netinet/in.h>

// inet_ntoa
# include <arpa/inet.h>

// accept, bind, setsockopt, recv, send, socket, socket option macro
# include <sys/socket.h>

// fcntl
# include <fcntl.h>

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

// Size of the receiving buffer being 512 as the max size of a packet defined in rfc 2812
# define MAX_REQUEST_LEN 512
// Also named backLog, use in listen to limit the number of simultaneous client connections
# define MAX_PENDING_CONNECTIONS 10

# define PASS "PASS"
# define NICK "NICK"
# define USER "USER"
# define PRIVMSG "PRIVMSG"
# define KILL "kill"
# define KICK "KICK"
# define QUIT "QUIT"
# define LIST "LIST"
# define PING "PING"
# define JOIN "JOIN"
# define OPER "OPER"
# define PART "PART"
# define MODE "MODE"
# define WHO "WHO"
# define NOTICE "NOTICE"

# define ERR_NEEDMOREPARAMS(NICK, CMD) ':' + _hostname + " 461 " + NICK + " " + CMD + " :Not enough parameters\r\n"
# define ERR_ALREADYREGISTRED(NICK) ':' + _hostname + " 462 " + NICK + " :You may not reregister\r\n"     
# define ERR_NOTREGISTERED ':' + _hostname + " : 451 :You have not registered\r\n"
# define ERR_NOPRIVILEGES(NICK) ':' + _hostname + " 481 " + NICK + " :Permission Denied- You're not an IRC operator\r\n"
# define ERR_NOSUCHNICK(REQUSER) ':' + _hostname + " 401 " + REQUSER + " :No such nick/channel\r\n"
# define ERR_CHANOPRIVSNEEDED(CHAN) ':' + _hostname + " 482 #" + CHAN + " :You're not channel operator\r\n"
# define ERR_NOSUCHCHANNEL(REQCHAN) ':' + _hostname + " 403 " + REQCHAN + " :No such channel\r\n"
# define ERR_NOTONCHANNEL(CHAN) ':' + _hostname +  " 442 #" + CHAN + " :You're not on that channel\r\n" 
# define ERR_NICKNAMEINUSE(NICK, NEWNICK) ':' + _hostname + " 433 " + NICK + " " + NEWNICK + " :Nickname is already in use\r\n"
# define RPL_ENDOFNAMES(USERC, NICK, CHAN) head(USERC) + "366 " + NICK + " #" + CHAN + " :End of /NAMES list\r\n"
# define ERR_PASSWDMISMATCH ':' + _hostname + " 464 :Password incorrect\r\n"
# define RPL_LIST(USERC, NICK, CHANNEL) head(input_user) + "322 " + NICK + " #" + CHANNEL
# define RPL_YOUREOPER(USERC) head(USERC) + "381 " + USERC->_nickname + " :You are now an IRC operator\r\n"
# define RPL_LISTEND(USERC, NICK) head(USERC) + "323 " + NICK + " :End of /LIST\r\n"
# define ERR_USERNOTINCHANNEL(NICK, USERTARGET, CHAN) ':' + _hostname + " 441 " + NICK + " " + USERTARGET + " #" + CHAN + " :They aren't on that channel\r\n"
# define MSG_KILL(USERC, REASON) head(USERC) + "KILL" + REASON + "\r\n"
# define MSG_QUIT(USERC, REASON) head(USERC) + "QUIT" + REASON + "\r\n"

# define PART_NOTICE(USERC, CHAN, REASON) head(USERC) + "PART #" + CHAN + REASON;

# define CMD_CLOSED_SOCKET true

# ifndef LINUX
#  define OS_NOEXCEPT _NOEXCEPT
#  define OS_SENDOPT 0
# else
#  define OS_NOEXCEPT _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT
#  define OS_SENDOPT MSG_NOSIGNAL
# endif

# include "User.hpp"
# include "log.hpp"
# include "Channel.hpp"

namespace irc
{

    class User;
    class Channel;

    class Server
    {

        private:
            const int *                         _password;
            unsigned int                        _passlength;
            const std::string                   _oper_log;
            const std::string                   _oper_pass;
            int                                 _listening_socket;
            fd_set                              _client_sockets; 
            fd_set                              _ready_sockets; 
            struct                              pending_socket; 
            std::map<int, pending_socket>       _unnamed_users;
            std::map<std::string, User *>       _connected_users;
            std::map<std::string, Channel *>    _running_channels;
            std::map<int, std::string *>        _pending_sends;
            std::set<int>                       _opened_sockets; 
            std::vector<User *>                 _to_kill_users;
            time_t                              _raw_start_time; 
            struct timeval                      _time_before_timeout; 
            char                                _main_buffer[MAX_REQUEST_LEN + 1];
            std::ofstream                       _log_file;
            std::string                         _hostname;

        public:
            typedef std::map<int, pending_socket>::iterator         unnamed_users_iterator_t;
            typedef std::map<std::string, User *>::iterator         connected_users_iterator_t;
            typedef std::map<std::string, Channel *>::iterator      running_channels_iterator_t;
            typedef std::map<int, std::string *>::iterator          pending_sends_iterator_t;

        private:
            struct CtorException : std::exception
            {
                const char * what() const OS_NOEXCEPT
                {
                    return "CtorException, check the logs!\n";
                }
            };

            struct partial_packet
            {
                std::string _recv;
                std::string _send;
            };

            struct pending_socket
            {
                std::string     nickname;
                std::string     username;
                bool            pass_check;
                partial_packet  _pending_data;

                pending_socket()
                    : nickname(std::string())
                    , username(std::string())
                    , pass_check(false)
                {
                }
            };
            
        public:
            Server( const char * port_number, const char * input_pass ); 
            ~Server();

            void set_password( const std::string new_password );

            bool user_acquired( const int fd );
            void loop( void );

            typedef std::string * (irc::Server::*command_function)( const int, const std::string, User * );

            void remove_empty_chan( Channel * target_chan );

        private:
            static std::map<const std::string, command_function>    _commands;
            void init_commands_map( void );

            std::string * cmd_pass( const int input_socket, const std::string command_line, User * input_user );
            std::string * cmd_nick( const int input_socket, const std::string command_line, User * input_user );
            std::string * cmd_user( const int input_socket, const std::string command_line, User * input_user );
            std::string * cmd_ping( const int input_socket, const std::string command_line, User * input_user ); 
            std::string * cmd_kill( const int input_socket, const std::string command_line, User * input_user );
            std::string * cmd_kick( const int input_socket, const std::string command_line, User * input_user );
            std::string * cmd_join( const int input_socket, const std::string command_line, User * input_user );
            std::string * cmd_quit( const int input_socket, const std::string command_line, User * input_user );
            std::string * cmd_list( const int input_socket, const std::string command_line, User * input_user );
            std::string * cmd_oper( const int input_socket, const std::string command_line, User * input_user );
            std::string * cmd_part( const int input_socket, const std::string command_line, User * input_user );
            std::string * cmd_privmsg(const int input_socket, const std::string command_line, User * input_user );
            std::string * cmd_mode(const int input_socket, const std::string command_line, User * input_user); 
            std::string * cmd_who(const int input_socket, const std::string command_line, User * input_user); 
            std::string * cmd_notice(const int input_socket, const std::string command_line, User *input_user);

            template < typename T >
                void cmd_caller( T identifier );

            void privmsg_hashtag_case(std::string command_line, User *input_user);
            void make_user_part(User * input_user, const std::string channel_name, const std::string reason);

            int * pass_hash(std::string input_pass );
            std::string * user_create(unnamed_users_iterator_t valid_unnamed_user);
            void disconnect_user(User * target_user); 
            void try_sending_data( void );

            std::string head(const User *input_user);
            void send_names(User * input_user, Channel * channel_target);
            std::string reply(const User * input_user ,  const char * code, std::string message) const;
            void quit_all_chan(User * target, std::string & reason);
    };

}

#endif
