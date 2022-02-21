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
        _ready_sockets = _client_sockets;
        local_time_before_timeout = _time_before_timeout;
        max_fd = *(_opened_sockets.rbegin());

        std::cout << "Waiting on select\n";
        number_of_ready_sockets = select(max_fd + 1, &_ready_sockets, NULL, NULL, &local_time_before_timeout);

        if (number_of_ready_sockets == -1)
        {
            std::cerr << "Error in select: " << strerror(errno) << "\n";
            break ; 
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
                --number_of_ready_sockets;

                if ((new_client_socket = accept(_listening_socket, new_client_address.ai_addr, &new_client_address.ai_addrlen)) != -1)
                {
# ifndef LINUX
                    int optval = 1;
                    if (setsockopt(_listening_socket, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval)) != 0)
                    {
                        close(new_client_socket);
                    }
                    else
# endif
                    {
                        fcntl(new_client_socket, F_SETFL, O_NONBLOCK);

                        FD_SET(new_client_socket, &_client_sockets);
                        _unnamed_users.insert(std::make_pair(new_client_socket, pending_socket()));
                        _opened_sockets.insert(new_client_socket);

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
                    std::cerr << "Accept fail: " << strerror(errno) << "\n";
                }
            }

            pending_socket_iterator = _unnamed_users.begin();
            while (number_of_ready_sockets > 0 && pending_socket_iterator != _unnamed_users.end())
            {
                tmp_pending_socket_iterator = pending_socket_iterator;
                ++pending_socket_iterator;

                if (FD_ISSET(tmp_pending_socket_iterator->first, &_ready_sockets))
                {
                    FD_CLR(tmp_pending_socket_iterator->first, &_ready_sockets);
                    --number_of_ready_sockets;

                    memset(_main_buffer, 0, MAX_REQUEST_LEN + 1);
                    byte_count = recv(tmp_pending_socket_iterator->first, _main_buffer, MAX_REQUEST_LEN, 0);
                    std::cout << "socket n'" << tmp_pending_socket_iterator->first << ". bytes received: " << byte_count << "\n";
                    if (byte_count == -1)
                    {
                        if (errno == EAGAIN)
                        {
                            continue ;
                        }
                        std::cout << "\nsocket n'" << tmp_pending_socket_iterator->first << " recv error: " << strerror(errno) << "\n";
                    }
                    else if (byte_count == 0)
                    {
                        std::cout << "\nsocket n'" << tmp_pending_socket_iterator->first << " is closed on client side.\n";
                    }
                    else
                    {
                        tmp_pending_socket_iterator->second._pending_data._recv.append(_main_buffer);
                        if (byte_count > 0)
                        {
                            std::cout << tmp_pending_socket_iterator->first << " Server <------| " <<  tmp_pending_socket_iterator->second._pending_data._recv;
                        }
                        cmd_caller(tmp_pending_socket_iterator);
                        continue ;
                    }
                    close(tmp_pending_socket_iterator->first);
                    _opened_sockets.erase(tmp_pending_socket_iterator->first);
                    FD_CLR(tmp_pending_socket_iterator->first, &_client_sockets);
                    _unnamed_users.erase(tmp_pending_socket_iterator);
                }
            }

            connected_user_iterator = _connected_users.begin();
            while (number_of_ready_sockets > 0 && connected_user_iterator != _connected_users.end())
            {
                tmp_connected_user_iterator = connected_user_iterator;
                ++connected_user_iterator;

                if (FD_ISSET(tmp_connected_user_iterator->second->_own_socket, &_ready_sockets))
                {
                    --number_of_ready_sockets;

                    memset(_main_buffer, 0, MAX_REQUEST_LEN + 1);
                    byte_count = recv(tmp_connected_user_iterator->second->_own_socket, _main_buffer, MAX_REQUEST_LEN, 0);
                    std::cout << "socket n'" << tmp_connected_user_iterator->second->_own_socket << ". bytes received: " << byte_count << "\n";
                    if (byte_count == -1)
                    {
                        if (errno == EAGAIN)
                        {
                            continue ;
                        }
                        std::cout << "\nsocket n'" << tmp_connected_user_iterator->second->_own_socket << " recv error: " << strerror(errno) << "\n";
                    }
                    else if (byte_count == 0)
                    {
                        std::cout << "\nsocket n'" << tmp_connected_user_iterator->second->_own_socket << " is closed on client side.\n";
                    }
                    else
                    {
                        tmp_connected_user_iterator->second->_pending_data._recv.append(_main_buffer);
                        if (byte_count > 0)
                        {
                            std::cout << tmp_connected_user_iterator->second->_own_socket << " ----> Server |" <<  tmp_connected_user_iterator->second->_pending_data._recv;
                        }

                        cmd_caller(tmp_connected_user_iterator->second);
                        continue ;
                    }
                    std::string reason = " :" + tmp_connected_user_iterator->second->_nickname + "\r\n";
                    quit_all_chan(tmp_connected_user_iterator->second, reason);
                    close(tmp_connected_user_iterator->second->_own_socket);
                    _opened_sockets.erase(tmp_connected_user_iterator->second->_own_socket);
                    FD_CLR(tmp_connected_user_iterator->second->_own_socket, &_client_sockets);
                    _connected_users.erase(tmp_connected_user_iterator);
                }
            }

            try_sending_data();
            std::vector<User *>::iterator      target_to_kill = _to_kill_users.begin();
            std::vector<User *>::iterator      tmp_target_to_kill;
            
            while(target_to_kill != _to_kill_users.end()) 
            {
                close((*target_to_kill)->_own_socket);
                _opened_sockets.erase((*target_to_kill)->_own_socket);
                _pending_sends.erase((*target_to_kill)->_own_socket);
                FD_CLR((*target_to_kill)->_own_socket, &_client_sockets);
                _connected_users.erase((*target_to_kill)->_nickname);
                delete *target_to_kill;
                tmp_target_to_kill = target_to_kill;
                ++target_to_kill;
            }
            _to_kill_users.clear();
        }
    }
}


void
irc::Server::try_sending_data( void )
{
    size_t                      end_of_line, last_end_of_line;
    int                         bytes_sent;

    pending_sends_iterator_t data_to_send_it = _pending_sends.begin();
    while (data_to_send_it != _pending_sends.end()) {
        last_end_of_line = end_of_line = 0;

        while ((end_of_line = data_to_send_it->second->find("\r\n", end_of_line)) != data_to_send_it->second->npos) {
            end_of_line += 2;
            last_end_of_line = end_of_line;
        }

        if (last_end_of_line != 0) {
            std::cout << "Server -->  " << data_to_send_it->first << ": " << data_to_send_it->second->data();
            if ((bytes_sent = send(data_to_send_it->first, data_to_send_it->second->data(), last_end_of_line, OS_SENDOPT)) == -1) {
                break ;
            } else if (static_cast<size_t>(bytes_sent) != last_end_of_line) {
                last_end_of_line = bytes_sent;
                break ;
            }
        }

        data_to_send_it->second->erase(0, last_end_of_line);

        if (data_to_send_it->second->empty()) {
            pending_sends_iterator_t tmp = data_to_send_it;
            ++data_to_send_it;
            _pending_sends.erase(tmp);
        } else {
            ++data_to_send_it;
        }
    }
}
