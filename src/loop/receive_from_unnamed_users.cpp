#include "Server.hpp"

void
irc::Server::receive_from_unnamed_users(int & number_of_ready_sockets)
{
    std::map<int, pending_socket>::iterator         pending_socket_iterator;
    std::map<int, pending_socket>::iterator         tmp_pending_socket_iterator;
    int                                             byte_count;

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
}
