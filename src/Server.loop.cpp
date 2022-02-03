#include "Server.hpp"

void
irc::Server::loop( void )
{
    int                                             number_of_ready_sockets;
    struct timeval                                  local_time_before_timeout;
    int                                             max_fd;
    std::map<std::string, irc::User *>::iterator    connected_user_iterator;
    std::map<std::string, irc::User *>::iterator    tmp_connected_user_iterator;
    std::map<int, pending_socket>::iterator         pending_socket_iterator;
    std::map<int, pending_socket>::iterator         tmp_pending_socket_iterator;
    std::set<int>::iterator                         opened_socket_iterator;
    int                                             new_client_socket;
    struct addrinfo                                 new_client_address;
    int                                             byte_count;

    for (;;)
    {
        // Set temporary variable that will get overwritten by select
        _ready_sockets = _client_sockets;
        local_time_before_timeout = _time_before_timeout;
        max_fd = *(_opened_sockets.rbegin());

        std::cout << "Waiting on select\n";
        number_of_ready_sockets = select(max_fd + 1, &_ready_sockets, NULL, NULL, &local_time_before_timeout);

        if (number_of_ready_sockets == -1)
        {
            // log select error
            std::cerr << "Error in select: " << strerror(errno) << "\n";
            break ; // WARNING: might have to retry
        }
        else if (number_of_ready_sockets == 0)
        {
            // log timeout
            std::cout << "Nothing received in last " << _time_before_timeout.tv_sec << " seconds\n";
            break ;
        }
        else
        {
            // If the listening socket is ready it means we have at least one connection to accept
            if (FD_ISSET(_listening_socket, &_ready_sockets))
            {
                --number_of_ready_sockets;

                if ((new_client_socket = accept(_listening_socket, new_client_address.ai_addr, &new_client_address.ai_addrlen)) != -1)
                {
                    int optval = 1;
                    if (setsockopt(_listening_socket, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval)) != 0)
                    {
                        // log socket did not set to needed options
                        close(new_client_socket);
                    }
                    else
                    {
                        // Set the new socket to non blocking, effectively protecting against a wrong ready-to-read from accept
                        fcntl(new_client_socket, F_SETFL, O_NONBLOCK);

                        FD_SET(new_client_socket, &_client_sockets);
                        _unnamed_users.insert(std::make_pair(new_client_socket, pending_socket()));
                        _opened_sockets.insert(new_client_socket);

                        // log connection accepted
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
                    }
                }
                else
                {
                    // log accept failed
                    std::cerr << "Accept fail: " << strerror(errno) << "\n";
                }
            }

            // For each opened socket, check if it's in the set of ready to read and act accordingly
            for (pending_socket_iterator = _unnamed_users.begin(); number_of_ready_sockets > 0 && pending_socket_iterator != _unnamed_users.end(); ++pending_socket_iterator)
            {
                // If the current socket is in the set of ready sockets
                if (FD_ISSET(pending_socket_iterator->first, &_ready_sockets))
                {
                    // Mark that we handled one of the ready sockets
                    --number_of_ready_sockets;

                    // Receive its data 
                    memset(_main_buffer, 0, MAX_REQUEST_LEN + 1);
                    byte_count = recv(pending_socket_iterator->first, _main_buffer, MAX_REQUEST_LEN, 0);
                    std::cout << "socket n'" << pending_socket_iterator->first << "bytes received: " << byte_count;
                    if (byte_count == -1)
                    {
                        if (errno == EAGAIN)
                        {
                            continue ;
                        }
                        std::cout << "socket n'" << pending_socket_iterator->first << " recv error: " << strerror(errno) << "\n";
                        close(pending_socket_iterator->first);
                        _opened_sockets.erase(pending_socket_iterator->first);
                        FD_CLR(pending_socket_iterator->first, &_client_sockets);
                        tmp_pending_socket_iterator = pending_socket_iterator;
                        ++tmp_pending_socket_iterator;
                        _unnamed_users.erase(pending_socket_iterator);
                        pending_socket_iterator = tmp_pending_socket_iterator;
                    }
                    std::cout << " raw input: [\n" << _main_buffer << "]\n";

                    cmd_caller(pending_socket_iterator->first);
                }
            }

            // For each opened socket, check if it's in the set of ready to read and act accordingly
            for (connected_user_iterator = _connected_users.begin(); number_of_ready_sockets > 0 && connected_user_iterator != _connected_users.end(); ++connected_user_iterator)
            {
                // If the current socket is in the set of ready sockets
                if (FD_ISSET(connected_user_iterator->second->_own_socket, &_ready_sockets))
                {
                    // Mark that we handled one of the ready sockets
                    --number_of_ready_sockets;

                    // Receive its data 
                    memset(_main_buffer, 0, MAX_REQUEST_LEN + 1);
                    byte_count = recv(connected_user_iterator->second->_own_socket, _main_buffer, MAX_REQUEST_LEN, 0);
                    std::cout << "socket n'" << connected_user_iterator->second->_own_socket << "bytes received: " << byte_count;
                    if (byte_count == -1)
                    {
                        if (errno == EAGAIN)
                        {
                            continue ;
                        }
                        std::cout << "socket n'" << connected_user_iterator->second->_own_socket << " recv error: " << strerror(errno) << "\n";
                        close(connected_user_iterator->second->_own_socket);
                        _opened_sockets.erase(connected_user_iterator->second->_own_socket);
                        FD_CLR(connected_user_iterator->second->_own_socket, &_client_sockets);
                        tmp_connected_user_iterator = connected_user_iterator;
                        ++tmp_connected_user_iterator;
                        _connected_users.erase(connected_user_iterator);
                        connected_user_iterator = tmp_connected_user_iterator;
                    }
                    std::cout << " raw input: [\n" << _main_buffer << "]\n";

                    cmd_caller(connected_user_iterator->second->_own_socket);
                }
            }
        }
    }
}
