#include "Server.hpp"

void
irc::Server::receive_from_connected_users(int & number_of_ready_sockets)
{
    std::map<std::string, irc::User *>::iterator    connected_user_iterator;
    std::map<std::string, irc::User *>::iterator    tmp_connected_user_iterator;
    int                                             byte_count;

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
            _pending_sends.erase(tmp_connected_user_iterator->second->_own_socket);
            FD_CLR(tmp_connected_user_iterator->second->_own_socket, &_client_sockets);
            _connected_users.erase(tmp_connected_user_iterator);
        }
    }
}
