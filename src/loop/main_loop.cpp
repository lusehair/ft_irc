#include "Server.hpp"

void
irc::Server::loop( void )
{
    int                                             number_of_ready_sockets;
    struct timeval                                  local_time_before_timeout;
    int                                             max_fd;
    std::set<int>::iterator                         opened_socket_iterator;

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
            std::cout << "Nothing received in last " << _time_before_timeout.tv_sec << " seconds\n";
            break ;
        }
        else
        {
            accept_connection(number_of_ready_sockets);

            receive_from_unnamed_users(number_of_ready_sockets);

            receive_from_connected_users(number_of_ready_sockets);

            try_sending_data();

            remove_killed_users();
        }
    }
}
