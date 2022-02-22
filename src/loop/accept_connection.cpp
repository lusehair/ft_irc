#include "Server.hpp"

void
irc::Server::accept_connection(int & number_of_ready_sockets)
{
    int                                             new_client_socket;
    struct addrinfo                                 new_client_address;

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
                std::cout << "Accepted conection\n";
            }
        }
        else
        {
            std::cerr << "Accept fail: " << strerror(errno) << "\n";
        }
    }
}
