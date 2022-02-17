#include "Server.hpp"

irc::Server::Server( const char * port_number, const char * input_pass )
    : _password(pass_hash(input_pass))
    , _passlength(strlen(input_pass))
    , _oper_log("admin")
    , _oper_pass("admin")
{
    int                 optval;
    struct addrinfo *   potential_addresses;
    struct addrinfo *   address_iterator;
    struct addrinfo     hints;

    { 
        _time_before_timeout.tv_sec = 60;
        _time_before_timeout.tv_usec = 0;
        FD_ZERO(&_client_sockets);
        memset(_main_buffer, 0, MAX_REQUEST_LEN + 1);
        memset(_ip_buffer, 0, INET6_ADDRSTRLEN);
        time(&_raw_start_time);
        _log_file.open (".log");
        LOG_IRC_START(_raw_start_time);  
    }

    { 
        memset(&hints, 0, sizeof(hints));
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = 0;
        hints.ai_addr = NULL;
        hints.ai_canonname = NULL;
        hints.ai_next = NULL;

        {
            int ret = getaddrinfo(NULL, port_number, &hints, &potential_addresses);
            if (ret != 0)
            {
                LOG_ERRGETADDRINFO(_raw_start_time, port_number);
                throw CtorException();
            }
            else
            {
                LOG_LISTPOTENTIALADDR(_raw_start_time, port_number);
            }
        } 

        { 
            for (address_iterator = potential_addresses; address_iterator != NULL; address_iterator = address_iterator->ai_next)
            {
                LOG_TRYBIDINGSOCKET(_raw_start_time, _listening_socket); 
                _listening_socket = socket(address_iterator->ai_family, address_iterator->ai_socktype, address_iterator->ai_protocol);

                if (_listening_socket == -1)
                {
                    LOG_INVALIDSOCKET(_raw_start_time, address_iterator->ai_addr->sa_data);
                    continue ;
                }

                optval = 1;
                if (setsockopt(_listening_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)))
                {
                    LOG_OPTSETFAIL(_raw_start_time, _listening_socket);
                    close(_listening_socket);
                    LOG_CLOSEFROMFAIL(_raw_start_time, _listening_socket); 
                    continue ;
                }

                if (bind(_listening_socket, address_iterator->ai_addr, address_iterator->ai_addrlen) != 0)
                {
                    LOG_BINDFAIL(_raw_start_time, _listening_socket); 
                    close(_listening_socket);
                    LOG_CLOSEFROMFAIL(_raw_start_time, _listening_socket); 
                    continue ;
                }
                else
                {
                    LOG_BINDSUCCESS(_raw_start_time, _listening_socket); 
                    memset(_ip_buffer, 0, INET6_ADDRSTRLEN);
                    
                    if (getnameinfo(address_iterator->ai_addr, address_iterator->ai_addrlen, _ip_buffer, sizeof(_ip_buffer), NULL, 0, 0) != 0)
                    {
                        LOG_CANNOTTRANSLATE(_raw_start_time);
                    }
                    else
                    {
                        LOG_SHOWIP(_raw_start_time, _ip_buffer);
                    }
                }

                LOG_TRYLISTENING(_raw_start_time, _listening_socket);
                if (listen(_listening_socket, MAX_PENDING_CONNECTIONS) != 0)
                {
                    LOG_LISTENFAILED(_raw_start_time, _listening_socket);
                }
                else
                {
                    LOG_LISTENSUCCESS(_raw_start_time, _listening_socket);
                    break ;
                }

                close(_listening_socket);
                LOG_CLOSEFROMFAIL(_raw_start_time, _listening_socket); 
            }
        } 
    }
    freeaddrinfo(potential_addresses);

    if (address_iterator == NULL)
    {
        LOG_NOMOREBIND(_raw_start_time);
        throw CtorException();
    }

    FD_SET(_listening_socket, &_client_sockets);
    _opened_sockets.insert(_listening_socket);

    init_commands_map();
} 


irc::Server::~Server()
{
    for(std::set<int>::iterator socket_iterator = _opened_sockets.begin(); socket_iterator != _opened_sockets.end(); ++socket_iterator)
    {
        LOG_CLOSINGFD(_raw_start_time, *socket_iterator);
        close(*socket_iterator);
    }

    for(std::map<std::string, User *>::iterator user_iterator = _connected_users.begin(); user_iterator != _connected_users.end(); ++user_iterator)
    {
        LOG_CLOSEUSER(_raw_start_time, user_iterator->first);        
        delete (user_iterator->second);
    }
    _connected_users.clear();
    LOG_CLOSEMAPUSER(_raw_start_time);
    FD_ZERO(&_client_sockets);
    LOG_CLOSEFDSOCKET(_raw_start_time);
    _opened_sockets.clear();
    LOG_CLOSEOPENSOCKET(_raw_start_time);
    _unnamed_users.clear();
    LOG_CLOSEPENDINGSOCKET(_raw_start_time);
    delete (_password);
    LOG_CLOSEPASS(_raw_start_time);
    _log_file.close();
}


void     irc::Server::disconnect_user(User * target_user)
{
        std::string tmp_nick = target_user->_nickname; 
        _opened_sockets.erase(target_user->_own_socket);
        FD_CLR(target_user->_own_socket, &_client_sockets);
        close(target_user->_own_socket); 
        delete(target_user);
        _connected_users.erase(target_user->_nickname);
} 


void
irc::Server::remove_empty_chan( Channel * target_chan )
{
    running_channels_iterator_t it = _running_channels.find(target_chan->get_name());
    if (it != _running_channels.end())
    {
        delete it->second;
        _running_channels.erase(it);
    }
}
