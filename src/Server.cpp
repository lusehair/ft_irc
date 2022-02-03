#include "Server.hpp"

irc::Server::Server( char * port_number )
    : _password("")
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
                LOG_TRYBIDINGSOCKET(_raw_start_time, address_iterator->ai_addr->sa_data); 
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
        //std::cout << socket << " has been closed\n";
    }

    FD_ZERO(&_client_sockets);
    std::cout << "The fd_set of client sockets has been emptied\n";
    _connected_users.clear();
    std::cout << "The map of users has been emptied\n";
    _opened_sockets.clear();
    std::cout << "The set of opened sockets has been emptied\n";
    _log_file.close();
}

void
irc::Server::set_password( const std::string new_password )
{
    _password = new_password;
}

bool
irc::Server::user_acquired(int fd)
{ (void)fd;
    // memset(_main_buffer, 0, MAX_REQUEST_LEN + 1);
    // recv(fd, _main_buffer, MAX_REQUEST_LEN, 0);
    // int i; 
    // std::string nick; 
    // std::string tmp(_main_buffer); 
    // size_t separate = tmp.find_first_of('@'); 
    // i = separate - 1;
    // while(tmp.compare("<"))
    //     i--;
    // i++;
    // tmp.copy((char*)nick.c_str(), (separate -1) - i, i); 
    // std::map<std::string, User>::iterator it; 

    // it = _connected_users.find(nick); 
    // if (it != _connected_users.end())
    // {
    //     close(fd); 
    //     // Method for closing fd on all fd set 
    //     _connected_users.erase(nick); 
    //     return false; 
    // }
    // // else if conditon for pass checking 

    // else 
    // {

    //     std::string username; 
    //     i = separate + 1; 
    //     while(tmp.compare(">"))
    //         i++;
    //     i--; 
    //     tmp.copy((char*)username.c_str(), (i - 1) - separate, i);
    //     FD_SET(new_client_socket, &_client_sockets);
    //     _connected_users.insert(std::make_pair(nick, User(nick, username, fd)));
    //     _opened_sockets.insert(fd);
        return true;
    // }
}

void
irc::Server::loop( void )
{
    int                                             number_of_ready_sockets;
    struct timeval                                  local_time_before_timeout;
    int                                             max_fd;
    // std::map<std::string, irc::User *>::iterator    opened_socket_iterator;
    // std::map<std::string, irc::User *>::iterator    target_user_iterator;
    std::set<int>::iterator                         opened_socket_iterator;
    int                                             new_client_socket;
    struct addrinfo                                 new_client_address;

    for (;;)
    {
        // Set temporary variable that will get overwritten by select
        _ready_sockets = _client_sockets;
        local_time_before_timeout = _time_before_timeout;
        max_fd = *(_opened_sockets.rbegin());

        //std::cout << "Waiting on select\n";
        number_of_ready_sockets = select(max_fd + 1, &_ready_sockets, NULL, NULL, &local_time_before_timeout);

        if (number_of_ready_sockets == -1)
        {
            // log select error
            LOG_SELECTERR(_raw_start_time);
            std::cerr << "Error in select: " << strerror(errno) << "\n";
            break ; // WARNING: might have to retry
        }
        else if (number_of_ready_sockets == 0)
        {
            // log timeout
            LOG_TIMEOUT;
            std::cout << "Nothing received in last " << _time_before_timeout.tv_sec << " seconds\n";
        break ;
        }
        else
        {
            // If the listening socket is ready it means we have at least one connection to accept
            if (FD_ISSET(_listening_socket, &_ready_sockets))
            {
                if ((new_client_socket = accept(_listening_socket, new_client_address.ai_addr, &new_client_address.ai_addrlen)) != -1)
                {
                    // Set the new socket to non blocking, effectively protecting against a wrong ready-to-read from accept
                    fcntl(new_client_socket, F_SETFL, O_NONBLOCK);

// TODO: verify if the password is correct and if the nick is unique. if so, let the programm continue. else, log wrong password or wrong nick (log in the function that checks for those)
// // If login info are correct and the user is registered in the database
// if (user_acquired(new_client_socket))
// {
// // Register the new user (in user_acquired function)
                    // _connected_users.insert(std::make_pair(std::string("toto" + std::to_string(new_client_socket)), User("toto", "toto", new_client_socket)));
                    FD_SET(new_client_socket, &_client_sockets);
                    _unnamed_users.insert(std::make_pair(new_client_socket, pending_socket()));
                    _opened_sockets.insert(new_client_socket);

    // // Send a welcome message to the new client
    // send(new_client_socket, "Sheeeeeeesh\n", strlen("Sheeeeeeesh\n"), MSG_DONTWAIT);

                    // log connection accepted
                    LOG_CONNECTACCEPT(_raw_start_time, new_client_socket);
                    //std::cout << "Accepted conection from : ";
                    memset(_ip_buffer, 0, INET6_ADDRSTRLEN);
                    if (getnameinfo(new_client_address.ai_addr, new_client_address.ai_addrlen, _ip_buffer, sizeof(_ip_buffer), NULL, 0, NI_NUMERICHOST) != 0)
                    {
                        //std::cout << "'could not translate the socket address'\n" ;
                    }
                    else
                    {
                        //std::cout << _ip_buffer << '\n';
                    }

    // // Notify other users of the newcomer
    // for (opened_socket_iterator = _connected_users.begin(); opened_socket_iterator != _connected_users.end(); ++opened_socket_iterator)
    // {
    //     send(opened_socket_iterator->second->_own_socket, "The new member is here!\n", strlen("The new member is here!\n"), MSG_DONTWAIT);
    // }
// }
                }
                else
                {
                    // log accept failed
                    LOG_CONNECTFAILED(_raw_start_time, new_client_socket);
                    std::cerr << "Accept fail: " << strerror(errno) << "\n";
                }
            }

            // For each opened socket, check if it's in the set of ready to read and act accordingly
            for (opened_socket_iterator = _opened_sockets.begin(); opened_socket_iterator != _opened_sockets.end() && number_of_ready_sockets > 0; ++opened_socket_iterator)
            {
                // If the current socket is in the set of ready sockets
                if (FD_ISSET(*opened_socket_iterator, &_ready_sockets))
                {
                    // Mark that we handled one of the ready sockets
                    --number_of_ready_sockets;

                    // Receive its data 
                    memset(_main_buffer, 0, MAX_REQUEST_LEN + 1);
                    recv(*opened_socket_iterator, _main_buffer, MAX_REQUEST_LEN, 0);

// TODO: recv errors handling
                    std::cout << "socket n'" << *opened_socket_iterator << " raw input: ";
                    std::cout << _main_buffer << "\n";

                    cmd_caller(*opened_socket_iterator);

// // TODO: parse the command and dont just send it to everyone
// // Send it raw to all users
// for (target_user_iterator = _opened_sockets.begin(); target_user_iterator != _opened_sockets.end(); ++target_user_iterator)
// {
//     send(target_user_iterator->second->_own_socket, _main_buffer, strlen(_main_buffer), MSG_DONTWAIT);
// }




                }
            }
        }
    }
}
