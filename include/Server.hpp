#ifndef SERVER_HPP
# define SERVER_HPP

// memset, strerro << "\n"r
#include <cstring>

// data storage
# include <string>
# include <map>

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

# define BUFFER_SIZE 512
# define MAX_PENDING_CONNECTIONS 10

namespace irc
{

    class Server
    {

        private:
            std::string             _password;
            int                     _listening_socket;
            fd_set                  _client_sockets; // for select parameters
            fd_set                  _ready_sockets; // for select return
            std::map<int, struct addrinfo> _connected_users; // int == fd ; addrinfo == User *
            std::map<void *, int>   _running_channels; // void * == Channel * ; int == Modes
            time_t                  _raw_start_time; // used for logs
            struct timeval          _time_before_timeout; // select timeout
            char                    _main_buffer[BUFFER_SIZE];
            char                    _ip_buffer[INET6_ADDRSTRLEN];

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
            Server( char * port_number );

            ~Server();

            void set_password( const std::string new_password );

            void loop( void );

    };

    Server::Server( char * port_number )
        : _password("")
    {
        int                 optval;
        struct addrinfo *   potential_addresses;
        struct addrinfo *   address_iterator;
        struct addrinfo     hints;

        { // Default value setting for member variables
            _time_before_timeout.tv_sec = 30;
            _time_before_timeout.tv_usec = 0;
            FD_ZERO(&_client_sockets);
            memset(_main_buffer, 0, BUFFER_SIZE);
            memset(_ip_buffer, 0, INET6_ADDRSTRLEN);
            time(&_raw_start_time);
            // log ctor start
        }

        { // Open a socket, bind and listen to it
            memset(&hints, 0, sizeof(hints));
            hints.ai_flags = AI_PASSIVE;
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = 0;
            hints.ai_addr = NULL;
            hints.ai_canonname = NULL;
            hints.ai_next = NULL;

            { // List potential addresses
                int ret = getaddrinfo(NULL, port_number, &hints, &potential_addresses);
                if (ret != 0)
                {
                    // log getaddrinfo fail intead of cerr print
                    std::cerr << gai_strerror(ret);
                    throw CtorException();
                }
                else
                {
                    // log List of potential addresses aquired
                }
            } // End of addresses listing

            { // Browse addresses until bind and listen success or no more addresses to try on
                for (address_iterator = potential_addresses; address_iterator != NULL; address_iterator = address_iterator->ai_next)
                {
                    // log try to bind to a socket
                    _listening_socket = socket(address_iterator->ai_family, address_iterator->ai_socktype, address_iterator->ai_protocol);

                    if (_listening_socket == -1)
                    {
                        // log invalid socket
                        std::cerr << "Socket opening failed: " << strerror(errno) << "\n";
                        continue ;
                    }

                    optval = 1;
                    if (setsockopt(_listening_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)))
                    {
                        // log option setting fail
                        close(_listening_socket);
                        std::cerr << "Socket option setting failed: " << strerror(errno) << "\n";
                        continue ;
                    }

                    if (bind(_listening_socket, address_iterator->ai_addr, address_iterator->ai_addrlen) != 0)
                    {
                        // log bind fail
                        std::cerr << "Binding failed: " << strerror(errno) << "\n";
                        close(_listening_socket);
                        continue ;
                    }
                    else
                    {
                        // log successful binding
                        std::cout << "Successful bind to ";

                        memset(_ip_buffer, 0, INET6_ADDRSTRLEN);
                        if (getnameinfo(address_iterator->ai_addr, address_iterator->ai_addrlen, _ip_buffer, sizeof(_ip_buffer), NULL, 0, 0) != 0)
                        {
                            std::cout << "'could not translate the socket address'";
                        }
                        else
                        {
                            std::cout << _ip_buffer << '\n';
                        }
                    }

                    // Can start listening and register the socked in the fd_set
                    if (listen(_listening_socket, MAX_PENDING_CONNECTIONS) != 0)
                    {
                        // log listen failed
                        std::cerr << "Cannot listen on the socket: " << strerror(errno) << "\n";
                    }
                    else
                    {
                        break ; // Break the loop on success
                    }

                    close(_listening_socket); // close and try again

                }
            } // End of address browsing loop
        } // Either listens to a socket or failed to with all potential addresses

        // No more use of potential_addresses
        freeaddrinfo(potential_addresses);

        // If no valid bind nor listen was made
        if (address_iterator == NULL)
        {
            // log no bind was possible and server can't start
            std::cerr << "Cannot bind to any tried address: " << strerror(errno) << "\n";
            throw CtorException();
        }

        FD_SET(_listening_socket, &_client_sockets);
    } // End of ctor, ready to loop

    Server::~Server()
    {
        close(_listening_socket);
        FD_ZERO(&_client_sockets);
    }

    bool
    Server::users_recquire(int fd)
    {
            int i; 
            std::string ret; 
            std::string tmp(raw_input);
            size_t separate = tmp.find_first_of('@'); 
            i = separate - 1;
            while(tmp.compare("<"))
                i--;
            i++;
            tmp.copy((char*)ret.c_str(), (separate -1) - i, i); 
            







    }


    void
    Server::set_password( const std::string new_password )
    {
        _password = new_password;
    }

    void
    Server::loop( void )
    {
        int                         number_of_ready_sockets;
        struct timeval              local_time_before_timeout;
        int                         max_fd;
        std::map<int, struct addrinfo>::iterator  user_iterator;
        std::map<int, struct addrinfo>::iterator  target_user;
        int                         new_client_socket;
        struct addrinfo             new_client_address;

        for (;;)
        {
            // Set temporary variable that will get overwritten by select
            _ready_sockets = _client_sockets;
            local_time_before_timeout = _time_before_timeout;

            max_fd = _connected_users.rbegin()->first;
            if (max_fd < _listening_socket)
            {
                max_fd = _listening_socket;
            }

            std::cout << "Waiting on select\n";
            number_of_ready_sockets = select(max_fd + 1, &_ready_sockets, NULL, NULL, &local_time_before_timeout);

            if (number_of_ready_sockets == -1)
            {
                // log select error
                std::cerr << "Error in select: " << strerror(errno) << "\n";
            }
            else if (number_of_ready_sockets == 0)
            {
                // log timeout
                std::cout << "Nothing received in last " << _time_before_timeout.tv_sec << " seconds\n";
                break ;
            }
            else
            {
                if (FD_ISSET(_listening_socket, &_ready_sockets))
                {
                    if ((new_client_socket = accept(_listening_socket, new_client_address.ai_addr, &new_client_address.ai_addrlen)) != -1)
                    {
                        fcntl(new_client_socket, F_SETFL, O_NONBLOCK);
                        FD_SET(new_client_socket, &_client_sockets);
                        _connected_users.insert(std::make_pair(new_client_socket, new_client_address));
                        send(new_client_socket, "Sheeeeeeesh\n", strlen("Sheeeeeeesh\n"), MSG_DONTWAIT);

                        std::cout << "Accepted conection from : ";
                        memset(_ip_buffer, 0, INET6_ADDRSTRLEN);
                        if (getnameinfo(new_client_address.ai_addr, new_client_address.ai_addrlen, _ip_buffer, sizeof(_ip_buffer), NULL, 0, NI_NUMERICHOST) != 0)
                        {
                            std::cout << "'could not translate the socket address'\n" ;
                        }
                        else
                        {
                            std::cout << _ip_buffer << '\n';
                        }

                        for (target_user = _connected_users.begin(); target_user != _connected_users.end(); ++target_user)
                        {
                            send(target_user->first, "The new member is here!\n", strlen("The new member is here!\n"), MSG_DONTWAIT);
                        }
                    }
                    else
                    {
                        // log accept failed
                        std::cerr << "Accept fail: " << strerror(errno) << "\n";
                    }
                }

                for (user_iterator = _connected_users.begin(); user_iterator != _connected_users.end() && number_of_ready_sockets > 0; ++user_iterator)
                {
                    if (FD_ISSET(user_iterator->first, &_ready_sockets))
                    {
                        --number_of_ready_sockets;

                        memset(_main_buffer, 0, BUFFER_SIZE);
                        recv(user_iterator->first, _main_buffer, BUFFER_SIZE, 0);

                        for (target_user = _connected_users.begin(); target_user != _connected_users.end(); ++target_user)
                        {
                            send(target_user->first, _main_buffer, strlen(_main_buffer), MSG_DONTWAIT);
                        }
                    }
                }
            }
        }
    }

}

#endif
