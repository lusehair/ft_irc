#include "Server.hpp"

irc::Server::Server( const char * port_number, const char * input_pass )
    : _password(pass_hash(input_pass))
{
    int                 optval;
    struct addrinfo *   potential_addresses;
    struct addrinfo *   address_iterator;
    struct addrinfo     hints;

    { // Default value setting for member variables
        _time_before_timeout.tv_sec = 60;
        _time_before_timeout.tv_usec = 0;
        FD_ZERO(&_client_sockets);
        memset(_main_buffer, 0, MAX_REQUEST_LEN + 1);
        memset(_ip_buffer, 0, INET6_ADDRSTRLEN);
        time(&_raw_start_time);
        _log_file.open (".log"); 
        // log ctor start
        LOG_IRC_START(_raw_start_time);  
        
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
                LOG_ERRGETADDRINFO(_raw_start_time, port_number);
                std::cerr << gai_strerror(ret);
                throw CtorException();
            }
            else
            {
                // log List of potential addresses aquired
                LOG_LISTPOTENTIALADDR(_raw_start_time, port_number);
            }
        } // End of addresses listing

        { // Browse addresses until bind and listen success or no more addresses to try on
            for (address_iterator = potential_addresses; address_iterator != NULL; address_iterator = address_iterator->ai_next)
            {
                // log try to bind to a socket
                LOG_TRYBIDINGSOCKET(_raw_start_time, _listening_socket); 
                _listening_socket = socket(address_iterator->ai_family, address_iterator->ai_socktype, address_iterator->ai_protocol);

                if (_listening_socket == -1)
                {
                    // log invalid socket
                    LOG_INVALIDSOCKET(_raw_start_time, address_iterator->ai_addr->sa_data);
                    std::cerr << "Socket opening failed: " << strerror(errno) << "\n";
                    continue ;
                }

                optval = 1;
                if (setsockopt(_listening_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)))
                {
                    // log option setting fail
                    LOG_OPTSETFAIL(_raw_start_time, _listening_socket);
                    std::cerr << "Socket option setting failed: " << strerror(errno) << "\n";
                    close(_listening_socket);
                    std::cout << _listening_socket << " listening socket has been closed\n";
                    continue ;
                }

                if (bind(_listening_socket, address_iterator->ai_addr, address_iterator->ai_addrlen) != 0)
                {
                    // log bind fail
                    LOG_BINDFAIL(_raw_start_time, _listening_socket); 
                    std::cerr << "Binding failed: " << strerror(errno) << "\n";
                    close(_listening_socket);
                    std::cout << _listening_socket << " listening socket has been closed\n";
                    continue ;
                }
                else
                {
                    // log successful binding
                    LOG_BINDSUCCESS(_raw_start_time, _listening_socket); 
                    //std::cout << "Successful bind to ";

                    memset(_ip_buffer, 0, INET6_ADDRSTRLEN);
                    if (getnameinfo(address_iterator->ai_addr, address_iterator->ai_addrlen, _ip_buffer, sizeof(_ip_buffer), NULL, 0, 0) != 0)
                    {
                        //std::cout << "'could not translate the socket address'";
                    }
                    else
                    {
                        //std::cout << _ip_buffer << '\n';
                    }
                }

                //log here try listen 
                LOG_TRYLISTENING(_raw_start_time, _listening_socket);
                // Can start listening and register the socked in the fd_set
                if (listen(_listening_socket, MAX_PENDING_CONNECTIONS) != 0)
                {
                    // log listen failed
                    LOG_LISTENFAILED(_raw_start_time, _listening_socket);
                    std::cerr << "Cannot listen on the socket: " << strerror(errno) << "\n";
                }
                else
                {
                    LOG_LISTENSUCCESS(_raw_start_time, _listening_socket);
                    break ; // Break the loop on success
                    //log 
                }

                close(_listening_socket); // close and try again
                std::cout << _listening_socket << " listening socket has been closed\n";

            }
        } // End of address browsing loop
    } // Either listens to a socket or failed to with all potential addresses

    // No more use of potential_addresses
    freeaddrinfo(potential_addresses);

    // If no valid bind nor listen was made
    if (address_iterator == NULL)
    {
        // log no bind was possible and server can't start
        LOG_NOMOREBIND(_raw_start_time);
        std::cerr << "Cannot bind to any tried address: " << strerror(errno) << "\n";
        throw CtorException();
    }

    FD_SET(_listening_socket, &_client_sockets);
    _opened_sockets.insert(_listening_socket);

    init_commands_map();
} // End of ctor, ready to loop

irc::Server::~Server()
{
    for(std::set<int>::iterator socket_iterator = _opened_sockets.begin(); socket_iterator != _opened_sockets.end(); ++socket_iterator)
    {
        LOG_CLOSINGFD(_raw_start_time, *socket_iterator);
        close(*socket_iterator);
    }

    for(std::map<std::string, User *>::iterator user_iterator = _connected_users.begin(); user_iterator != _connected_users.end(); ++user_iterator)
    {
        // LOG_CLOSINGFD(_raw_start_time, *socket_iterator);
        std::cout << "User: " << user_iterator->first << " deletion.\n";
        delete (user_iterator->second);
    }
    _connected_users.clear();
    std::cout << "The map of users has been emptied\n";

    FD_ZERO(&_client_sockets);
    std::cout << "The fd_set of client sockets has been emptied\n";
    _opened_sockets.clear();
    std::cout << "The set of opened sockets has been emptied\n";
    _unnamed_users.clear();
    std::cout << "The map of pending sockets has been emptied\n";
    delete (_password);
    std::cout << "The password has been freed\n";
    _log_file.close();
}

// void
// irc::Server::set_password( const std::string new_password )
// {
//     _password = new_password;
// }
